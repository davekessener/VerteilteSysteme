package vs.app.client;

import java.rmi.NotBoundException;
import java.rmi.Remote;
import java.rmi.RemoteException;
import java.rmi.registry.LocateRegistry;

import dave.util.log.Logger;
import dave.util.log.Severity;
import javafx.beans.property.Property;
import javafx.beans.property.SimpleObjectProperty;
import javafx.scene.Node;
import javafx.scene.control.Alert.AlertType;
import javafx.scene.control.ButtonType;
import vs.app.common.Component;
import vs.app.common.Status;
import vs.app.ui.QuickAlert;

public class RemoteComponent<T extends Remote> implements Component
{
	private final Property<T> mBean;
	private final String mService;
	private final RemoteUI mUI;
	private final Property<Status> mConnected;
	private String mHost;
	private int mPort;
	
	public RemoteComponent(String service, String host, int port)
	{
		mBean = new SimpleObjectProperty<>();
		mService = service;
		mUI = new RemoteUI(host, port);
		mConnected = new SimpleObjectProperty<>(null);
		
		mUI.onUpdate(this::update);
		
		mConnected.addListener((ob, o, n) -> mUI.setStatus(n));
		
		update(host, port);
	}
	
	public Property<Status> connectedProperty( ) { return mConnected; }
	public Property<T> serviceProperty( ) { return mBean; }
	
	@Override
	public Node getUI()
	{
		return mUI.getUI();
	}
	
	// the proxy has encountered an error -> try to retrieve another
	// stub from the registry
	public void refresh( )
	{
		try
		{
			retrieve(mHost, mPort);
		}
		catch(RemoteException | NotBoundException e)
		{
			mConnected.setValue(Status.SEARCHING);
		}
	}
	
	private void update(String host, int port)
	{
		try
		{
			retrieve(host, port);
			
			mUI.setRemote(mHost = host, mPort = port);
		}
		catch(RemoteException | NotBoundException e)
		{
			LOG.log(Severity.ERROR, "Could not connected to [%s]:%d! (%s)", host, port, e.getMessage());

			QuickAlert.show(AlertType.ERROR, String.format("Failed to connect to [%s]:%d\n\"%s\"", host, port, e.getMessage()), ButtonType.OK);
		}
	}

	@SuppressWarnings("unchecked")
	private void retrieve(String host, int port) throws RemoteException, NotBoundException
	{
		mBean.setValue((T) LocateRegistry.getRegistry(host, port).lookup(mService));
		
		mConnected.setValue(Status.CONNECTED);
	}
	
	private static final Logger LOG = Logger.get("remote");
}
