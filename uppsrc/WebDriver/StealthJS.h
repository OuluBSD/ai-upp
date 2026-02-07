#ifndef _WebDriver_StealthJS_h_
#define _WebDriver_StealthJS_h_

#include <Core/Core.h>

NAMESPACE_UPP

inline const char* GetStealthJS() {
	return R"(
		// Remove webdriver property
		Object.defineProperty(navigator, 'webdriver', {
			get: () => undefined,
			set: () => {}
		});

		// Mock plugins
		const mockPlugins = [
			{ name: 'PDF Viewer', filename: 'internal-pdf-viewer', description: 'Portable Document Format' },
			{ name: 'Chrome PDF Viewer', filename: 'mhjfbmdgcfjbbpaeojofohoefgiehjai', description: '' },
			{ name: 'Native Client', filename: 'internal-nacl-plugin', description: '' }
		];
		mockPlugins.length = 3;
		Object.defineProperty(navigator, 'plugins', { get: () => mockPlugins });

		// Mock languages
		Object.defineProperty(navigator, 'languages', { get: () => ['en-US', 'en'] });

		// Mock hardwareConcurrency
		Object.defineProperty(navigator, 'hardwareConcurrency', { get: () => 4 });

		// Fix WebGL detection
		const getParameter = WebGLRenderingContext.prototype.getParameter;
		WebGLRenderingContext.prototype.getParameter = function(parameter) {
			if (parameter === 37445) return 'Intel Inc.';
			if (parameter === 37446) return 'Intel Iris OpenGL Engine';
			return getParameter(parameter);
		};
	)";
}

END_UPP_NAMESPACE

#endif
