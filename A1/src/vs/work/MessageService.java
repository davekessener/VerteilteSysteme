package vs.work;

import java.rmi.Remote;
import java.rmi.RemoteException;

public interface MessageService extends Remote
{
	public abstract String nextMessage(String cid) throws RemoteException;
	public abstract void newMessage(String cid, String msg) throws RemoteException;
}
