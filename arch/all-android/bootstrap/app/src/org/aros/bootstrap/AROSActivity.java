package org.aros.bootstrap;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.graphics.Canvas;
import android.os.Bundle;
import android.os.Looper;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import java.lang.String;
import java.nio.IntBuffer;

public class AROSActivity extends Activity
{
	static final int ID_ERROR_DIALOG = 0;
	static final int ID_ALERT_DIALOG = 1;

    private CharSequence errStr;
    private DisplayView rootView;
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
    	Log.d("AROS.UI", "Activity created");

    	AROSBootstrap app = (AROSBootstrap) getApplication();

    	rootView = new DisplayView(this, app);
    	super.onCreate(savedInstanceState);
        setContentView(rootView);

    	// Notify application object about our creation
    	app.ui = this;
    }

    @Override
    public void onDestroy()
    {
    	Log.d("AROS.UI", "Activity destroyed");
    	
    	AROSBootstrap app = (AROSBootstrap) getApplication();
    	app.ui = null;
    	
    	super.onDestroy();
    }

    public void DisplayError(String text)
    {
    	errStr = text;
    	showDialog(ID_ERROR_DIALOG);
    }

    public void DisplayAlert(String text)
    {
    	errStr = text;
    	showDialog(ID_ALERT_DIALOG);

    	Looper.loop();
    }

    public BitmapView GetBitmap(int id)
    {
    	return rootView.GetBitmap(id);
    }

    public Dialog onCreateDialog(int id)
    {
		AlertDialog.Builder b = new AlertDialog.Builder(this);
		DialogInterface.OnClickListener okEvent; 
    	
    	switch (id)
    	{
    	case ID_ERROR_DIALOG:
    		b.setTitle(R.string.error);
    		okEvent = new DialogInterface.OnClickListener()
        	{	
    			@Override
    			public void onClick(DialogInterface dialog, int which)
    			{
    				System.exit(0);
    			}
    		};
    		break;

    	case ID_ALERT_DIALOG:
    		b.setTitle(R.string.guru);
    		okEvent = new DialogInterface.OnClickListener()
        	{
    			@Override
    			public void onClick(DialogInterface dialog, int which)
    			{
    				// TODO: break run loop and return
    				System.exit(0);
    			}
    		};
    		break;
    	
    	default:
    		return null;
    		
    	}
    	b.setMessage(errStr);
    	b.setCancelable(false);
    	b.setPositiveButton(R.string.ok, okEvent);

    	return b.create();
    }
}

// This is our display class
class DisplayView extends ViewGroup
{
	private AROSBootstrap main;
	private BitmapView bitmap;

	public DisplayView(Context context, AROSBootstrap app)
	{
		super(context);
		main = app;

		bitmap = new BitmapView(context, app);
		addView(bitmap);
	}

	public BitmapView GetBitmap(int id)
	{
		return bitmap;
	}

	@Override
	protected void onLayout(boolean c, int left, int top, int right, int bottom)
	{
		if (!c)
			return;

		main.DisplayWidth = right - left;
		main.DisplayHeight = bottom - top;
		Log.d("AROS", "Screen size set: " + main.DisplayWidth + "x" + main.DisplayHeight);

		main.Boot();
	}
}

// This is our bitmap class
class BitmapView extends View
{
	private AROSBootstrap main;

	public BitmapView(Context context, AROSBootstrap app)
	{
		super(context);
		main = app;
	}

	@Override
	protected void onDraw(Canvas c)
	{
		BitmapData bm = main.Bitmap;

		if (bm == null)
		{
			c.drawColor(0);
		}
		else
		{
			int stride = bm.BytesPerRow / 4;
			IntBuffer ib = bm.Pixels.asIntBuffer();
			int[] Data = new int[ib.capacity()];
			ib.get(Data);
		
			c.drawBitmap(Data, 0, stride, 0, 0, main.DisplayWidth, main.DisplayHeight, false, null);
		}
	}
}
