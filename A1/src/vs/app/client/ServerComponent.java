package vs.app.client;

import java.util.stream.Collectors;

import javafx.scene.Node;
import vs.app.common.Component;
import vs.work.ChatEngine;

public class ServerComponent implements Component
{
	private final ChatEngine mServer;
	private final ServerUI mUI;
	
	public ServerComponent(ChatEngine s)
	{
		mServer = s;
		mUI = new ServerUI();
		
		mServer.indexProperty().addListener(o -> mUI.setText(mServer.entries().collect(Collectors.joining("\n"))));
	}

	@Override
	public Node getUI()
	{
		return mUI.getUI();
	}
}
