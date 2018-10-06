package vs.app.client;

import java.util.stream.Collectors;

import javafx.beans.property.BooleanProperty;
import javafx.beans.property.SimpleBooleanProperty;
import javafx.scene.Node;
import vs.app.common.Component;
import vs.work.ChatEngine;

public class ServerComponent implements Component
{
	private final ChatEngine mServer;
	private final ServerUI mUI;
	private final BooleanProperty mConnected;
	
	public ServerComponent(ChatEngine s)
	{
		mServer = s;
		mUI = new ServerUI();
		mConnected = new SimpleBooleanProperty(true);
		
		mServer.indexProperty().addListener(o -> mUI.setText(mServer.entries().collect(Collectors.joining("\n"))));
	}

	@Override
	public Node getUI()
	{
		return mUI.getUI();
	}

	@Override
	public BooleanProperty connectedProperty()
	{
		return mConnected;
	}
}
