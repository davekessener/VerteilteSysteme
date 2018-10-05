package vs.app.client;

import java.util.function.Consumer;

import javafx.scene.Node;
import javafx.scene.control.Button;
import javafx.scene.control.TextField;
import javafx.scene.input.KeyCode;
import javafx.scene.layout.HBox;
import javafx.scene.layout.Priority;

public class RedaktionUI
{
	private final HBox mRoot;
	private final TextField mText;
	private final Button mSend;
	
	public RedaktionUI( )
	{
		mRoot = new HBox();
		mText = new TextField();
		mSend = new Button("Send");
		
		mRoot.setSpacing(4);
		
		HBox.setHgrow(mText, Priority.ALWAYS);
		
		mRoot.getChildren().addAll(mText, mSend);
	}
	
	public Node getUI( ) { return mRoot; }
	
	public void setOnSend(Consumer<String> cb)
	{
		mSend.setOnAction(e -> cb.accept(mText.getText()));
		mText.setOnKeyPressed(e -> {
			if(e.getCode() == KeyCode.ENTER)
			{
				cb.accept(mText.getText());
			}
		});
	}
}
