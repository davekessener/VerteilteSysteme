package vs.play.exec;

import java.io.Serializable;
import java.rmi.Remote;
import java.rmi.RemoteException;

public interface RemoteExecutor extends Remote
{
	public abstract <T extends Serializable> T run(Task<T> task) throws RemoteException;
}
