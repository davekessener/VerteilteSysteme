package vs.app;

import java.util.ArrayList;
import java.util.List;

import dave.util.Actor;
import javafx.scene.Node;
import javafx.scene.Scene;
import javafx.scene.layout.AnchorPane;
import javafx.stage.Stage;
import vs.app.common.Component;
import vs.app.ui.AppUI;

public abstract class App implements Actor
{
	private final Stage mPrimary;
	private final AppUI mUI;
	private final List<Component> mComponents;
	
	public App(Stage primary)
	{
		mPrimary = primary;
		mUI = new AppUI();
		mComponents = new ArrayList<>();
		
		AnchorPane root = new AnchorPane();
		Node ui = mUI.getUI();
		Scene s = new Scene(root, 640, 480);
		
		root.getChildren().add(ui);

		AnchorPane.setLeftAnchor(ui, 0d);
		AnchorPane.setRightAnchor(ui, 0d);
		AnchorPane.setTopAnchor(ui, 0d);
		AnchorPane.setBottomAnchor(ui, 0d);
		
		mPrimary.setScene(s);
		
		mPrimary.show();
	}
	
	protected void addComponent(Component c)
	{
		mComponents.add(c);
		mUI.add(c.getUI());
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
