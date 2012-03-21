#pragma once

namespace CliArduino {
	void Init(XShell& shell);
}

void CliInit(XShell& shell) {
	CliArduino::Init(shell);
}
