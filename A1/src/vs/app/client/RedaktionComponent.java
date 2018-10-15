package vs.app.client;

import java.rmi.RemoteException;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.DelayQueue;
import java.util.concurrent.Delayed;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

import dave.util.log.Logger;
import dave.util.log.Severity;
import javafx.beans.property.BooleanProperty;
import javafx.beans.property.Property;
import javafx.beans.property.SimpleBooleanProperty;
import javafx.scene.Node;
import javafx.scene.control.ButtonType;
import javafx.scene.control.Alert.AlertType;
import vs.app.common.Component;
import vs.app.common.InternalRemoteException;
import vs.app.ui.QuickAlert;
import vs.util.Properties;
import vs.work.MessageService;

public class RedaktionComponent implements Component
{
	private final Property<MessageService> mServer;
	private final Property<String> mID;
	private final RedaktionUI mUI;
	private final BooleanProperty mConnected;
	private final ExecutorService mAsync;
	private final BlockingQueue<Attempt> mAttempts;
	
	public RedaktionComponent(Property<MessageService> chat)
	{
		mServer = chat;
		mConnected = new SimpleBooleanProperty(true);
		mID = Properties.get(Properties.CLIENT_ID);
		mUI = new RedaktionUI();
		mAsync = Executors.newSingleThreadExecutor();
		mAttempts = new DelayQueue<>();
		
		mUI.setOnSend(this::sendMessage);
	}
	
	public BooleanProperty connectedProperty( ) { return mConnected; }
	
	@Override
	public void focus( )
	{
		mUI.focus();
	}

	@Override
	public Node getUI()
	{
		return mUI.getUI();
	}
	
	@Override
	public void start( )
	{
		mAsync.submit(this::run);
	}
	
	@Override
	public void stop( )
	{
		mAsync.shutdownNow();
	}
	
	private void sendMessage(String msg)
	{
		if(mServer.getValue() == null)
		{
			QuickAlert.show(AlertType.ERROR, "No remote service selected!", ButtonType.OK);
		}
		else
		{
			mAttempts.add(new Attempt(msg));
			mUI.resetInput();
		}
	}
	
	private int run( ) throws InterruptedException
	{
		while(true)
		{
			Attempt a = mAttempts.take();
			
			try
			{
				mServer.getValue().newMessage(mID.getValue(), a.message);
			}
			catch(InternalRemoteException e)
			{
				LOG.log(Severity.ERROR, "Invalid: %s", e.getMessage());
				
				QuickAlert.show(AlertType.ERROR, "Invalid operation!\n\"" + e.getMessage() + "\"", ButtonType.OK);
			}
			catch(RemoteException e)
			{
				Property<Number> tries = Properties.get(Properties.TRANSMIT_RETRIES);
				
				if(a.attempt < tries.getValue().intValue())
				{
					mAttempts.add(new Attempt(a));
					
					LOG.log(Severity.WARNING, "Failed to reach server (%d); retrying ...", a.attempt); 
				}
				else
				{
					mConnected.set(false);
					
					LOG.log(Severity.ERROR, "Can't send message to server: %s", e.getMessage());
					
					QuickAlert.show(AlertType.ERROR, "Message send failure:\n\"" + e.getMessage() + "\"", ButtonType.OK);
				}
			}
		}
	}
	
	private static class Attempt implements Delayed
	{
		public final int attempt;
		public final String message;
		private final long start;
		
		public Attempt(String msg)
		{
			attempt = 0;
			message = msg;
			start = System.currentTimeMillis();
		}
		
		public Attempt(Attempt previous)
		{
			attempt = previous.attempt + 1;
			message = previous.message;
			start = System.currentTimeMillis() + 100 * (1 << (attempt - 1));
		}

		@Override
		public int compareTo(Delayed o)
		{
			Attempt a = (Attempt) o;
			
			return Long.compare(start, a.start);
		}

		@Override
		public long getDelay(TimeUnit u)
		{
			return u.convert(start - System.currentTimeMillis(), TimeUnit.MILLISECONDS);
		}
	}
	
	private static final Logger LOG = Logger.get("redaktion");
}
