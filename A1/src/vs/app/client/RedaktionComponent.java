package vs.app.client;

import javafx.scene.Node;
import vs.app.common.Component;

public class RedaktionComponent implements Component
{
	private final RedaktionUI mUI;
	
	public RedaktionComponent( )
	{
		mUI = new RedaktionUI();
	}
	
	@Override
	public Node getUI()
	{
		return mUI.getUI();
	}
}
