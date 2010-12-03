package org.aros.bootstrap;

import org.aros.bootstrap.R;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.Environment;
import java.lang.String;

public class AROSBootstrap extends Activity
{
	static final int ID_ERROR_DIALOG = 0;
	static final int ID_GURU_DIALOG = 1;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
     
        // Get external storage path 
        String extdir = Environment.getExternalStorageDirectory().getAbsolutePath();

        int rc = Start(extdir + "/AROS");
        DisplayError("Bootstrap exited with rc" + rc);
    }

    public void DisplayError(String text)
    {
    	errStr = text;
    	showDialog(ID_ERROR_DIALOG);
    }

    public void DisplayGuru(CharSequence text)
    {
    	errStr = text;
    	showDialog(ID_GURU_DIALOG);
    	// TODO: enter run loop here
    }
    
    public Dialog onCreateDialog(int id)
    {
		AlertDialog.Builder b = new AlertDialog.Builder(this);
    	
    	switch (id)
    	{
    	case ID_ERROR_DIALOG:
    		b.setTitle(R.string.error);
    		break;

    	case ID_GURU_DIALOG:
    		b.setTitle(R.string.guru);
    		break;
    	
    	default:
    		return null;
    		
    	}
    	b.setMessage(errStr);
    	b.setCancelable(false);
    	b.setPositiveButton(R.string.ok, new DialogInterface.OnClickListener()
    	{	
			@Override
			public void onClick(DialogInterface dialog, int which)
			{
				AROSBootstrap.this.finish();
			}
		});

    	return b.create();
    }

    public native int Start(String dir);

    static
    {
        System.loadLibrary("AROSBootstrap");
    }

    private CharSequence errStr;
}
