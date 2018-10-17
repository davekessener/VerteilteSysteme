package vs.work;

import java.rmi.RemoteException;
import java.util.function.Consumer;

import dave.util.DirtyObject;

public class MessageServiceProxy extends DirtyObject implements MessageService
{
	private final Consumer<MessageServiceProxy> mCallback;
	private MessageService mRemote;
	
	public MessageServiceProxy(Consumer<MessageServiceProxy> cb)
	{
		mCallback = cb;
		mRemote = null;
		
		use();
	}
	
	public void set(MessageService s)
	{
		mRemote = s;
		
		clean();
	}

	@Override
	public String nextMessage(String cid) throws RemoteException
	{
		if(dirty())
		{
			mCallback.accept(this);
		}
		
		try
		{
			if(mRemote == null)
				throw new RemoteException("No connection");
			
			return mRemote.nextMessage(cid);
		}
		catch(RemoteException e)
		{
			use();
			
			throw e;
		}
	}

	@Override
	public void newMessage(String cid, String msg) throws RemoteException
	{
		if(dirty())
		{
			mCallback.accept(this);
		}
		
		try
		{
			if(mRemote == null)
				throw new RemoteException("No connection");
			
			mRemote.newMessage(cid, msg);
		}
		catch(RemoteException e)
		{
			use();
			
			throw e;
		}
	}
}
