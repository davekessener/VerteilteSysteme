package vs.app.client;

import java.rmi.RemoteException;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

import dave.util.log.Logger;
import dave.util.log.Severity;
import javafx.beans.property.BooleanProperty;
import javafx.beans.property.Property;
import javafx.beans.property.SimpleBooleanProperty;
import javafx.scene.Node;
import javafx.scene.control.Alert;
import javafx.scene.control.Alert.AlertType;
import javafx.scene.control.ButtonType;
import vs.app.common.Component;
import vs.app.ui.QuickAlert;
import vs.util.Properties;
import vs.work.MessageService;

public class RezeptionComponent implements Component
{
	private final Property<MessageService> mServer;
	private final RezeptionUI mUI;
	private final ScheduledExecutorService mAsync;
	private final BooleanProperty mConnected;
	private Future<?> mRunning;
	private long mFirstDisconnect;
	
	public RezeptionComponent(Property<MessageService> chat)
	{
		mServer = chat;
		mUI = new RezeptionUI();
		mAsync = Executors.newSingleThreadScheduledExecutor();
		mConnected = new SimpleBooleanProperty(true);
		mFirstDisconnect = -1;
		
		mConnected.addListener(o -> {
			if(mConnected.get())
			{
				mFirstDisconnect = -1;
			}
		});
		
		mUI.setOnUpdate(this::manualUpdate);
		mUI.activeProperty().addListener((ob, o, n) -> {
			if(n)
			{
				mRunning = mAsync.scheduleAtFixedRate(this::automaticUpdate, 0, mUI.getFrequency(), TimeUnit.MILLISECONDS);
			}
			else if(mRunning != null)
			{
				mRunning.cancel(false);
				mRunning = null;
			}
		});
		mUI.connectedProperty().bind(mConnected);
	}
	
	public BooleanProperty connectedProperty( ) { return mConnected; }
	
	private void manualUpdate( )
	{
		try
		{
			if(!retrieveNextMessage())
			{
				Alert a = new Alert(AlertType.INFORMATION, "No new messages.", ButtonType.OK);
				
				a.showAndWait();
			}
		}
		catch(RemoteException e)
		{
			LOG.log(Severity.ERROR, "Connection to server failed: %s", e.getMessage());
			mConnected.setValue(false);
			
			Alert a = new Alert(AlertType.ERROR, "Can't connect to server:\n\"" + e.getMessage() + "\"", ButtonType.OK);
			
			a.showAndWait();
		}
	}
	
	private void automaticUpdate( )
	{
		try
		{
			retrieveNextMessage();
		}
		catch(RemoteException e)
		{
			LOG.log(Severity.ERROR, "Connection to server failed: %s", e.getMessage());
			mConnected.setValue(false);
			
			Property<Number> timeout = Properties.get(Properties.SERVER_FAILURE_TIMEOUT);
			long t = System.currentTimeMillis();
			
			if(mFirstDisconnect == -1) mFirstDisconnect = t;
			
			if(t - mFirstDisconnect > timeout.getValue().intValue())
			{
				mUI.activeProperty().set(false);
				mFirstDisconnect = -1;
				
				QuickAlert.show(AlertType.ERROR, "Connection to server lost!", ButtonType.OK);
			}
		}
	}
	
	private synchronized boolean retrieveNextMessage( ) throws RemoteException
	{
		Property<String> id = Properties.get(Properties.CLIENT_ID);
		String msg = mServer.getValue().nextMessage(id.getValue());
		
		mConnected.set(true);
		
		if(msg != null) mUI.addMessage(msg);
		
		return msg != null;
	}
	
	@Override
	public Node getUI()
	{
		return mUI.getUI();
	}

	private static final Logger LOG = Logger.get("rezeption");
}
