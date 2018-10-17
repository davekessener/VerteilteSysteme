package vs.app;

import java.util.ArrayList;
import java.util.List;

import dave.util.Actor;
import javafx.beans.property.Property;
import javafx.beans.property.SimpleObjectProperty;
import javafx.scene.Node;
import javafx.scene.Scene;
import javafx.scene.layout.AnchorPane;
import javafx.stage.Stage;
import vs.app.common.Component;
import vs.app.common.Status;
import vs.app.ui.AppUI;

public class App implements Actor
{
	private final Stage mPrimary;
	private final AppUI mUI;
	private final List<Component> mComponents;
	private final Property<Status> mConnected;
	
	public App(Stage primary)
	{
		mPrimary = primary;
		mUI = new AppUI();
		mComponents = new ArrayList<>();
		mConnected = new SimpleObjectProperty<>(null);
		
		AnchorPane root = new AnchorPane();
		Node ui = mUI.getUI();
		Scene s = new Scene(root, 1280, 720);
		
		root.getChildren().add(ui);
		
		AnchorPane.setLeftAnchor(ui, 0d);
		AnchorPane.setRightAnchor(ui, 0d);
		AnchorPane.setTopAnchor(ui, 0d);
		AnchorPane.setBottomAnchor(ui, 0d);
		
		mPrimary.setScene(s);
		
		mPrimary.show();
	}
	
	public Property<Status> connectedProperty( ) { return mConnected; }
	
	public void addComponent(Component c)
	{
		mComponents.add(c);
		
		mUI.add(c.getUI());
		
		c.focus();
	}

	@Override
	public void start()
	{
		mComponents.forEach(Component::start);
	}

	@Override
	public void stop()
	{
		mComponents.forEach(Component::stop);
	}
}
