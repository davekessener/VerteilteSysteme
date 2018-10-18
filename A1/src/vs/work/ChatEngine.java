package vs.work;

import java.rmi.RemoteException;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import java.util.stream.Stream;

import dave.util.Actor;
import dave.util.log.Logger;
import dave.util.log.Severity;
import javafx.beans.property.Property;
import javafx.beans.property.SimpleLongProperty;
import vs.app.common.InternalRemoteException;
import vs.util.FrequencyQueue;
import vs.util.InetAddressValidator;

public class ChatEngine implements MessageService, MessageServiceConfiguration, Actor
{
	private final ScheduledExecutorService mAsync;
	private final BlockingQueue<Entry> mMessages;
	private final Map<String, Client> mClients;
	private int mTimeoutPeriod, mTimeoutCount, mTimeoutDuration;
	private int mCapacity, mForgetTime;
	private final Property<Number> mNextID;
	
	public ChatEngine( )
	{
		mTimeoutPeriod = DEF_TIMEOUT_PERIOD;
		mTimeoutCount = DEF_TIMEOUT_COUNT;
		mTimeoutDuration = DEF_TIMEOUT_DURATION;
		mCapacity = DEF_CAPACITY;
		mForgetTime = DEF_FORGET;
		
		mAsync = Executors.newSingleThreadScheduledExecutor();
		mMessages = new LinkedBlockingQueue<>();
		mClients = new HashMap<>();
		mNextID = new SimpleLongProperty(0);
	}
	
	public Property<Number> indexProperty( ) { return mNextID; }
	public Stream<String> entries( ) { return mMessages.stream().map(Entry::toString); }

	@Override
	public void setTimeout(int p, int n, int d) throws RemoteException
	{
		if(p < 1 || p < 1 || d < 1)
		{
			LOG.log(Severity.ERROR, "Tried to configure timeout with invalid parameters! (%d %d %d)", p, n, d);
			
			throw new RemoteException("Invalid timeout config! (" + p + ", " + n + ", " + d + ")");
		}
		
		mTimeoutPeriod = p;
		mTimeoutCount = n;
		mTimeoutDuration = d;
	}

	@Override
	public void setQueueCapacity(int n) throws RemoteException
	{
		if(n < 1)
		{
			LOG.log(Severity.ERROR, "Tried to configure capacity with invalid value! (%d)", n);
			
			throw new RemoteException("Invalid capacity config! (" + n + ")");
		}
		
		mCapacity = n;
	}

	@Override
	public void setForgetTime(int t) throws RemoteException
	{
		if(t < 1)
		{
			LOG.log(Severity.ERROR, "Tried to configure forget time with invalid value! (%d)", t);
			
			throw new RemoteException("Invalid forget config! (" + t + ")");
		}
		
		mForgetTime = t;
	}
	
	@Override
	public void start( )
	{
	}
	
	@Override
	public void stop( )
	{
		mAsync.shutdown();
	}

	@Override
	public String nextMessage(String cid) throws RemoteException
	{
		String next = null;

		if(cid == null)
		{
			LOG.log(Severity.WARNING, "Received 'null' client id!");
			
			throw new InternalRemoteException("Client ID missing!");
		}
		
		synchronized(mClients)
		{
			Client c = get(cid);
			
			if(!c.empty) synchronized(mMessages)
			{
				for(Entry e : mMessages)
				{
					if(e.id > c.last) // retrieve the next message
					{
						next = e.message;
						c.last = e.id;
						
						break;
					}
				}
				
				c.empty = (next == null);
			}
		}
		
		return next;
	}

	@Override
	public void newMessage(String cid, String msg) throws RemoteException
	{
		if(cid == null)
		{
			LOG.log(Severity.WARNING, "Received 'null' client id!");
			
			throw new InternalRemoteException("Client ID missing!");
		}
		
		if(msg == null)
		{
			LOG.log(Severity.WARNING, "Received 'null' message from %s!", cid);
			
			throw new InternalRemoteException("Message missing!");
		}
		
		if(!InetAddressValidator.getInstance().isValid(cid))
		{
			LOG.log(Severity.WARNING, "Client ID '%s' is not a valid IP addresss!", cid);
			
			throw new InternalRemoteException(cid + " is not a valid IP address!");
		}
		
		synchronized(mClients)
		{
			Client c = get(cid);
			long t = System.currentTimeMillis();
			
			// if the client has been timed out (and the timeout has not expired yet)
			// OR the client is actively spamming, time the client out.
			if((c.timeout > 0 && t - c.timeout < mTimeoutDuration) || !c.history.next())
			{
				c.timeout = t;
				
				LOG.log(Severity.WARNING, "Client %s is being timed out for %dms! (%d+ messages in %ss)",
						cid, mTimeoutDuration, mTimeoutCount, 
						(mTimeoutPeriod % 1000 == 0 ? ("" + mTimeoutPeriod) : String.format("%.2f", mTimeoutPeriod / 1000.0)));
				
				throw new InternalRemoteException("" + mTimeoutDuration + "ms timeout for spam!");
			}
			
			mClients.values().stream().forEach(cc -> cc.empty = false);
		}
		
		synchronized(mMessages)
		{
			Entry e = new Entry(cid, msg);
			
			while(mMessages.size() >= mCapacity)
			{
				mMessages.poll();
			}
			
			mMessages.add(e);
			
			mNextID.setValue(mNextID.getValue().longValue() + 1);
			
			LOG.log("Client %s sent new message '%s'", cid, msg);
		}
	}
	
	private Client get(String cid)
	{
		Client c = mClients.get(cid);
		
		if(c == null)
		{
			mClients.put(cid, c = new Client());
		}
		
		// if deletion has been scheduled and could not be
		// canceled (because it has already been executed)
		// re-insert client
		if(c.deleter != null && !c.deleter.cancel(false))
		{
			mClients.put(cid, c);
		}
		
		c.deleter = mAsync.schedule(() -> {
			synchronized(mClients)
			{
				mClients.remove(cid);
			}
		}, mForgetTime, TimeUnit.MILLISECONDS);
		
		return c;
	}
	
	private final class Entry
	{
		public final long id;
		public final String client;
		public final String message;
		
		public Entry(String cid, String msg)
		{
			id = mNextID.getValue().longValue();
			client = cid;
			message = String.format("%04d %15s: %-80s %s", id, client, msg, FORMAT.format(new Date()));
		}
		
		@Override
		public String toString( )
		{
			return message;
		}
	}
	
	private final class Client
	{
		public final FrequencyQueue history;
		public Future<?> deleter;
		public long timeout;
		public long last;
		public boolean empty;
		
		public Client( )
		{
			this.history = new FrequencyQueue(mTimeoutPeriod, mTimeoutCount);
			this.deleter = null;
			this.timeout = -1;
			this.last = -1;
			this.empty = false;
		}
	}
	
	public static final int DEF_TIMEOUT_PERIOD = 1 * 1000;
	public static final int DEF_TIMEOUT_COUNT = 10;
	public static final int DEF_TIMEOUT_DURATION = 10 * 1000;
	public static final int DEF_CAPACITY = 15;
	public static final int DEF_FORGET = 30 * 1000;
	
	private static final DateFormat FORMAT = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSS");
	private static final Logger LOG = Logger.get("chat");
}
