package vs.app.ui;

import java.util.function.Predicate;


public class IntegerTextField extends FilteredTextField
{
	public IntegerTextField(Predicate<Integer> f, int def)
	{
		super("" + def, txt -> {
			try
			{
				int v = Integer.parseInt(txt);
				
				if(!f.test(v))
					throw new NumberFormatException();
				
				return true;
			}
			catch(NumberFormatException e)
			{
				return false;
			}
		});
	}
}
