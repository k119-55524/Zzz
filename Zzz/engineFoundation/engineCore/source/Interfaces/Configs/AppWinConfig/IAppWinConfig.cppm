
export module IAppWinConfig;

import Result;
import ioZaml;
import ZamlProcessor;

export namespace zzz::core
{
	export class IAppWinConfig
	{
	protected:
		virtual Result<> Configure(std::shared_ptr<ZamlProcessor> zamlData) = 0;
	};
}