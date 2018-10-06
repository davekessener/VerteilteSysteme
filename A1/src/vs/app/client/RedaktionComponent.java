package vs.app.client;

import java.rmi.RemoteException;

import dave.util.log.Logger;
import dave.util.log.Severity;
import javafx.beans.property.BooleanProperty;
import javafx.beans.property.Property;
import javafx.beans.property.SimpleBooleanProperty;
import javafx.scene.Node;
import javafx.scene.control.ButtonType;
import javafx.scene.control.Alert;
import javafx.scene.control.Alert.AlertType;
import vs.app.common.Component;
import vs.app.common.InternalRemoteException;
import vs.util.Properties;
import vs.work.MessageService;

public class RedaktionComponent implements Component
{
	private final MessageService mServer;
	private final RedaktionUI mUI;
	private final BooleanProperty mConnected;
	
	public RedaktionComponent(MessageService chat)
	{
		mServer = chat;
		mConnected = new SimpleBooleanProperty(true);
		mUI = new RedaktionUI();
		
		mUI.setOnSend(this::sendMessage);
	}
	
	private void sendMessage(String msg)
	{
		Property<String> id = Properties.get(Properties.CLIENT_ID);
		int tries = Properties.<Number>get(Properties.TRANSMIT_RETRIES).getValue().intValue();
		RemoteException error = null;
		
		do
		{
			if(tries == 0)
			{
				mConnected.set(false);
				
				LOG.log(Severity.ERROR, "Can't send message to server: %s", error.getMessage());
				
				Alert a = new Alert(AlertType.ERROR, "Message send failure:\n\"" + error.getMessage() + "\"", ButtonType.OK);
				
				a.showAndWait();
				
				break;
			}
			
			--tries;
			
			try
			{
				mServer.newMessage(id.getValue(), msg);
				mUI.resetInput();
				error = null;
			}
			catch(RemoteException e)
			{
				error = e;
				
				if(e instanceof InternalRemoteException)
				{
					tries = 0;
				}
			}
		}
		while(error != null);
	}
	
	@Override
	public BooleanProperty connectedProperty( )
	{
		return mConnected;
	}
	
	@Override
	public Node getUI()
	{
		return mUI.getUI();
	}
	
	private static final Logger LOG = Logger.get("redaktion");
}
