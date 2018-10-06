package vs.app.ui;

import java.util.function.Predicate;

import dave.util.log.Logger;
import dave.util.log.Severity;
import javafx.beans.property.Property;
import javafx.beans.property.SimpleIntegerProperty;
import javafx.scene.control.Alert;
import javafx.scene.control.Alert.AlertType;
import javafx.scene.control.ButtonType;
import javafx.scene.control.TextField;
import javafx.scene.input.KeyCode;

public class IntegerTextField extends TextField
{
	private final Predicate<Integer> mFilter;
	private final Property<Number> mValue;
	
	public IntegerTextField(Predicate<Integer> f, int v)
	{
		mFilter = f;
		mValue = new SimpleIntegerProperty(v);
		
		this.setText("" + v);
		
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
		
		try
		{
			int v = Integer.parseInt(txt);
			
			if(!mFilter.test(v))
				throw new NumberFormatException("Invalid number!");
			
			mValue.setValue(v);
		}
		catch(NumberFormatException e)
		{
			Logger.DEFAULT.log(Severity.WARNING, "Tried to enter invalid number '%s'!", txt);
			
			Alert a = new Alert(AlertType.ERROR, "'" + txt + "' is not a valid number!", ButtonType.OK);
			
			a.showAndWait();
			
			this.setText("" + mValue.getValue());
		}
	}
	
	public Property<Number> valueProperty( ) { return mValue; }
}
