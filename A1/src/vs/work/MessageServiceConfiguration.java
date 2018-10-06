package vs.work;

import java.rmi.Remote;
import java.rmi.RemoteException;

public interface MessageServiceConfiguration extends Remote
{
	public abstract void setTimeout(int p, int n, int d) throws RemoteException;
	public abstract void setQueueCapacity(int n) throws RemoteException;
	public abstract void setForgetTime(int t) throws RemoteException;
}
