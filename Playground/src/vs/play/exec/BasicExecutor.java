package vs.play.exec;

import java.io.Serializable;
import java.rmi.RemoteException;

public class BasicExecutor implements RemoteExecutor
{
	@Override
	public <T extends Serializable> T run(Task<T> task) throws RemoteException
	{
		return task.execute();
	}
}
