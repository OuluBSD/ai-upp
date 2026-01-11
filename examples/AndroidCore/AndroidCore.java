package org.upp.AndroidCore;

/**
 * Native Core functions from package AndroidCore.
 */
public class AndroidCore
{
	private AndroidCore() {}

	// Native stuff - C/C++
	public static native String processString(String input);
	public static native String stringOperations();
	public static native String formatOperations();
}