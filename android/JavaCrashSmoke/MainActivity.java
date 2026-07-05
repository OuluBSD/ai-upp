package org.upp.JavaCrashSmoke;

import android.app.Activity;
import android.os.Bundle;

public class MainActivity extends Activity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        System.out.println("MainActivity.onCreate starting...");
        
        // Let's trigger a crash via level1 -> level2
        level1();
    }

    private void level1() {
        level2();
    }

    private void level2() {
        String s = null;
        s.length(); // Throws NullPointerException
    }
}
