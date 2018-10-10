package vs.app.client;

import java.rmi.RemoteException;

import javafx.scene.Node;
import vs.app.common.Component;
import vs.work.MessageServiceConfiguration;

public class ConfigurationComponent implements Component
{
	private final MessageServiceConfiguration mConfig;
	private final ConfigurationUI mUI;
	
	public ConfigurationComponent(MessageServiceConfiguration s)
	{
		mConfig = s;
		mUI = new ConfigurationUI();

		mUI.timeoutPeriodProperty().addListener(o -> updateTimeout());
		mUI.timeoutCountProperty().addListener(o -> updateTimeout());
		mUI.timeoutDurationProperty().addListener(o -> updateTimeout());
		mUI.capacityProperty().addListener((ob, o, n) -> update(() -> mConfig.setQueueCapacity(n.intValue())));
		mUI.forgetProperty().addListener((ob, o, n) -> update(() -> mConfig.setForgetTime(n.intValue())));
	}
	
	private void updateTimeout( )
	{
		update(() -> mConfig.setTimeout(
					mUI.timeoutPeriodProperty().getValue().intValue(),
					mUI.timeoutCountProperty().getValue().intValue(),
					mUI.timeoutDurationProperty().getValue().intValue()));
	}
	
	private void update(Updater u)
	{
		try
		{
			u.update();
		}
		catch(RemoteException e)
		{
			e.printStackTrace();
		}
	}

	@Override
	public Node getUI()
	{
		return mUI.getUI();
	}
	
	private static interface Updater { void update( ) throws RemoteException; }
}
