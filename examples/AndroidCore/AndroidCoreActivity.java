package org.upp.AndroidCore;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;

public class AndroidCoreActivity extends Activity
{
	private EditText editText;
	private TextView outputText;

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);

		// Create main layout
		LinearLayout mainLayout = new LinearLayout(this);
		mainLayout.setOrientation(LinearLayout.VERTICAL);

		// Create input field
		editText = new EditText(this);
		editText.setHint("Enter text here");
		mainLayout.addView(editText);

		// Create buttons
		Button button1 = new Button(this);
		button1.setText("Process with Core");
		button1.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				processWithCore();
			}
		});
		mainLayout.addView(button1);

		Button button2 = new Button(this);
		button2.setText("String Operations");
		button2.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				stringOperations();
			}
		});
		mainLayout.addView(button2);

		Button button3 = new Button(this);
		button3.setText("Format Operations");
		button3.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				formatOperations();
			}
		});
		mainLayout.addView(button3);

		Button button4 = new Button(this);
		button4.setText("Test CoreStringProcessor");
		button4.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				testCoreStringProcessor();
			}
		});
		mainLayout.addView(button4);

		// Create output text view
		ScrollView scrollView = new ScrollView(this);
		outputText = new TextView(this);
		outputText.setText("Output will appear here...\n");
		scrollView.addView(outputText);
		mainLayout.addView(scrollView);

		setContentView(mainLayout);
	}

	private void processWithCore()
	{
		String input = editText.getText().toString();
		if (input.isEmpty()) {
			input = "Default input";
		}

		String result = AndroidCore.processString(input);
		outputText.append("Core processing result: " + result + "\n");
	}

	private void stringOperations()
	{
		String result = AndroidCore.stringOperations();
		outputText.append("String operations result: " + result + "\n");
	}

	private void formatOperations()
	{
		String result = AndroidCore.formatOperations();
		outputText.append("Format operations result: " + result + "\n");
	}

	private void testCoreStringProcessor()
	{
		String input = editText.getText().toString();
		if (input.isEmpty()) {
			input = "Test input";
		}

		CoreStringProcessor processor = new CoreStringProcessor(input);
		String processed = processor.process();
		String reversed = processor.getReversed();
		int length = processor.getLength();

		outputText.append("CoreStringProcessor result:\n");
		outputText.append("  Processed: " + processed + "\n");
		outputText.append("  Reversed: " + reversed + "\n");
		outputText.append("  Length: " + length + "\n");
	}

	static {
		// For Android version less than 5.0 we need to specific standard library
		// If you have a crash uncomment library that you specific in your Android Builder
		// specification.

		// System.loadLibrary("stlport_shared");
		// System.loadLibrary("gnustl_shared");
		// System.loadLibrary("c++_shared");

		// In this place we are loading native libraries (In revers dependency order).
		// IMPORTANT: Native library always has upp package name!!!
		System.loadLibrary("AndroidCore");
		System.loadLibrary("Core");
	}
}