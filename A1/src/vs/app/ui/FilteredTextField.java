package vs.app.ui;

import java.util.function.Consumer;
import java.util.function.Predicate;

import dave.util.log.Logger;
import dave.util.log.Severity;
import javafx.beans.property.Property;
import javafx.beans.property.SimpleStringProperty;
import javafx.scene.control.ButtonType;
import javafx.scene.control.TextField;
import javafx.scene.control.Alert.AlertType;
import javafx.scene.input.KeyCode;

public class FilteredTextField extends TextField
{
	private final Predicate<String> mCallback;
	private final Property<String> mValue;
	private final Consumer<String> mErrorCallback;
	
	public FilteredTextField(String def, Predicate<String> f)
	{
		this(def, f, v -> {
			Logger.DEFAULT.log(Severity.WARNING, "Tried to enter an invalid value: '%s'", v);
			QuickAlert.show(AlertType.ERROR, "Invalid value: '" + v + "'!", ButtonType.OK);
		});
	}
	
	public Property<String> valueProperty( ) { return mValue; }
	
	public FilteredTextField(String def, Predicate<String> f, Consumer<String> onerr)
	{
		super(def);
		
		mCallback = f;
		mValue = new SimpleStringProperty();
		mErrorCallback = onerr;

		this.focusedProperty().addListener((ob, o, n) -> {
			if(n)
			{
				this.selectAll();
			}
			else
			{
				validate();
			}
		});
		
		this.setOnKeyPressed(e -> {
			if(e.getCode() == KeyCode.ENTER)
			{
				this.selectAll();
				validate();
			}
		});
	}

	private void validate( )
	{
		String txt = this.getText();
		
		if(mCallback.test(txt))
		{
			mValue.setValue(txt);
		}
		else
		{
			this.setText(mValue.getValue());
			
			mErrorCallback.accept(txt);
		}
	}
}
