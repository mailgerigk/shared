#include <optional>
#include <string>
#include <iostream>

namespace // Library code
{
	template<typename Value, typename Error>
	struct Result
	{
		template<typename Ok, typename Err>
		void match(Ok ok, Err err)
		{
			if (value.has_value())
			{
				ok(value.value());
			}
			else
			{
				err(error.value());
			}
		}

		Result(Value value)
			: value(value)
			, error(std::nullopt)
		{
		}
		Result(Error error)
			: value(std::nullopt)
			, error(error)
		{
		}

	private:
		std::optional<Value> value;
		std::optional<Error> error;
	};

	template<typename Value>
	struct value_t
	{
		value_t(Value value)
			: value(value)
		{

		}
		template<typename Error>
		operator Result<Value, Error>()
		{
			return Result<Value, Error>(value);
		}
	private:
		Value value;
	};

	template<typename Error>
	struct error_t
	{
		error_t(Error error)
			: error(error)
		{

		}
		template<typename Value>
		operator Result<Value, Error>()
		{
			return Result<Value, Error>(error);
		}
	private:
		Error error;
	};

	template<typename Value>
	value_t<Value> ok(Value value)
	{
		return { value };
	}

	template<typename Error>
	error_t<Error> err(Error error)
	{
		return { error };
	}

	template<typename Value, typename Error>
	struct Process
	{
		virtual Result<Value, Error> getResult() = 0;
	};
}

// usage code
namespace GetSaveGame
{
	struct Error
	{
		enum class Reason
		{
			FileNotFound,
			CloudServerDown,
			NoNetwork,
		};

		Reason reason;
		std::string context;
	};

	struct SaveGame
	{
		int foo;
	};

	struct Process : public ::Process<SaveGame*, Error>
	{
		Result<SaveGame*, Error> getResult() override
		{
			bool networkFailed = rand() % 5;
			if (networkFailed)
			{
				return err(Error{ Error::Reason::NoNetwork, "You pleb got no network" });
			}

			bool cloudFailed = rand() % 5;
			if (cloudFailed)
			{
				return err(Error{ Error::Reason::CloudServerDown, "Could not reach cloud server at address 127.0.0.1" });
			}

			bool localFailed = rand() % 5;
			if (localFailed)
			{
				return err(Error{ Error::Reason::FileNotFound, "Could not find file..." });
			}

			return ok(new SaveGame{ 42 });
		}
	};
}

int main()
{
	GetSaveGame::Process process;
	process.getResult().match(
		[](GetSaveGame::SaveGame* saveGame)
		{
			std::cout << "Sucessfully got save game" << std::endl;
		},
		[](GetSaveGame::Error error)
		{
			switch (error.reason)
			{
			case GetSaveGame::Error::Reason::CloudServerDown:
				std::cout << "Error: " << error.context << std::endl;
				break;
			case GetSaveGame::Error::Reason::FileNotFound:
				std::cout << "Error: " << error.context << std::endl;
				break;
			case GetSaveGame::Error::Reason::NoNetwork:
				std::cout << "Error: No network available" << std::endl;
				break;
			}
		}
	);
}