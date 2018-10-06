package vs.app.ui;

import java.util.ArrayList;
import java.util.List;

import javafx.geometry.Insets;
import javafx.scene.Node;
import javafx.scene.Parent;
import javafx.scene.layout.GridPane;
import javafx.scene.layout.Priority;

public class AppUI
{
	private final GridPane mRoot;
	private final List<Node> mSegments;
	
	public AppUI( )
	{
		mRoot = new GridPane();
		mSegments = new ArrayList<>();
		
		mRoot.setPadding(new Insets(8, 8, 8, 8));
		mRoot.setVgap(6);
	}
	
	public Parent getUI( ) { return mRoot; }
	
	public void add(Node ui)
	{
		mRoot.add(ui, 0, mSegments.size());
		
		GridPane.setHgrow(ui, Priority.ALWAYS);
		
		mSegments.add(ui);
	}
}
