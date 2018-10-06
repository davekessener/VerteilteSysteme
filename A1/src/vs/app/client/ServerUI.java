package vs.app.client;

import javafx.scene.Node;
import javafx.scene.control.TextArea;
import javafx.scene.layout.GridPane;
import javafx.scene.layout.Priority;
import javafx.scene.text.Font;

public class ServerUI
{
	private final TextArea mRoot;
	
	public ServerUI( )
	{
		mRoot = new TextArea();
		
		mRoot.setEditable(false);
		mRoot.setFont(Font.font("Courier new", 12));
		
		GridPane.setHgrow(mRoot, Priority.ALWAYS);
		GridPane.setVgrow(mRoot, Priority.ALWAYS);
	}
	
	public Node getUI( ) { return mRoot; }
	
	public void setText(String s)
	{
		mRoot.setText(s);
	}
}
