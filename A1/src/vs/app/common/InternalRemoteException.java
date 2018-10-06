package vs.app.common;

import java.rmi.RemoteException;

public class InternalRemoteException extends RemoteException
{
	private static final long serialVersionUID = 8711457472844505299L;
	
	public InternalRemoteException(String msg)
	{
		super(msg);
	}
}
