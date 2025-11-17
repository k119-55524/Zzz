
export module IAppWinConfig;

import Result;
import ioZaml;

export namespace zzz::core
{
	export class IAppWinConfig
	{
	protected:
		virtual Result<> Configure(std::shared_ptr<zamlNode> zamlConfig) = 0;
	};
}