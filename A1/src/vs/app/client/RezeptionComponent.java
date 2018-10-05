package vs.app.client;

import javafx.scene.Node;
import vs.app.common.Component;

public class RezeptionComponent implements Component
{
	private final RezeptionUI mUI;
	
	public RezeptionComponent( )
	{
		mUI = new RezeptionUI();
	}
	
	@Override
	public Node getUI()
	{
		return mUI.getUI();
	}
}
