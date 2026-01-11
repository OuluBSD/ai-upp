package org.upp.AndroidCore;

/**
 * Core string processor class which uses Upp::Core functionality natively (c/c++).
 */
public class CoreStringProcessor
{
	/**
	 * Field use by MemoryManager to store pointer to c++ object.
	 * If you want to use MemoryManager with your class make sure you have this field
	 * in your class.
	 */
	private long nativeAdress = 0;

	/**
	 * We override finalize method, because we need to destroy native c++ object when
	 * there is not more reference to Java object. This method is called by default
	 * by garbage collector.
	 */
	@Override
	protected void finalize() throws Throwable
	{
		nativeFinalize();
		super.finalize();
	}

	public CoreStringProcessor(String input)
	{
		construct(input);
	}

	// Native stuff - C/C++
	public native String process();
	public native String getReversed();
	public native int getLength();

	private native void construct(String input);
	private native void nativeFinalize();
}