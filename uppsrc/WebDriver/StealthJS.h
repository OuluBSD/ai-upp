#ifndef _WebDriver_StealthJS_h_
#define _WebDriver_StealthJS_h_

#include <Core/Core.h>

NAMESPACE_UPP

inline const char* GetStealthJS() {
	return R"(
		// Ensure navigator.webdriver is undefined (standard Firefox behavior)
		try {
			Object.defineProperty(navigator, 'webdriver', {
				get: () => undefined,
				set: () => {},
				configurable: true
			});
		} catch (e) {}

		// Firefox should NOT have window.chrome
		if (window.chrome) {
			delete window.chrome;
		}
		
		// Some detectors check for this
		if (window.cdc_adoQtmx083120120533123) {
			delete window.cdc_adoQtmx083120120533123;
		}

		// Mock plugins
		const mockPlugins = [
			{ name: 'PDF Viewer', filename: 'internal-pdf-viewer', description: 'Portable Document Format' },
			{ name: 'Chrome PDF Viewer', filename: 'mhjfbmdgcfjbbpaeojofohoefgiehjai', description: '' },
			{ name: 'Native Client', filename: 'internal-nacl-plugin', description: '' }
		];
		mockPlugins.length = 3;
		try {
			Object.defineProperty(navigator, 'plugins', { get: () => mockPlugins });
		} catch (e) {}

		// Mock languages
		try {
			Object.defineProperty(navigator, 'languages', { get: () => ['en-US', 'en'] });
		} catch (e) {}

		// Mock hardwareConcurrency
		try {
			Object.defineProperty(navigator, 'hardwareConcurrency', { get: () => 4 });
		} catch (e) {}

		// Fix WebGL detection
		const getParameter = WebGLRenderingContext.prototype.getParameter;
		WebGLRenderingContext.prototype.getParameter = function(parameter) {
			if (parameter === 37445) return 'Intel Inc.';
			if (parameter === 37446) return 'Intel Iris OpenGL Engine';
			return getParameter(parameter);
		};
		
		// Hide automation clues in functions
		const toString = Function.prototype.toString;
		Function.prototype.toString = function() {
			if (this === Function.prototype.toString) return 'function toString() { [native code] }';
			if (this === navigator.webdriver) return 'function get webdriver() { [native code] }';
			return toString.call(this);
		};
		
		// Mask specific geckodriver/marionette markers if they leak
		// window.navigator.webdriver = false; // Removed: direct assignment makes it enumerable
	)";
}

END_UPP_NAMESPACE

#endif
