#include <optional>
#include <string>
#include <iostream>
#include <functional>
#include <variant>

#include <ctime>

namespace // Library code
{
	template<typename Value, typename Error>
	struct Result
	{
		Result(Value value)
			: has_value(false)
			, valueOrError(value)
		{
		}
		Result(Error error)
			: has_value(false)
			, valueOrError(error)
		{
		}

		template<typename Ok, typename Err>
		void match(Ok ok, Err err)
		{
			if (has_value)
			{
				ok(std::get<Value>(valueOrError));
			}
			else
			{
				err(std::get<Error>(valueOrError));
			}
		}

	private:
		bool has_value;
		std::variant<Value, Error> valueOrError;
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
}

namespace // base classes
{
	struct ProcessInterface
	{
		virtual bool doWork() = 0;
		virtual void callOnExit() = 0;
	};

	template<typename Value, typename Error>
	struct Process : public ProcessInterface
	{
		using ResultType = Result<Value, Error>;
		using BaseType = Process<Value, Error>;

		virtual ResultType getResult() = 0;
	};

	struct ProcessMgr
	{
		void add(ProcessInterface* process)
		{
			proccess.push_back(process);
		}
		void update()
		{
			for (auto i = proccess.begin(); i != proccess.end(); ++i)
			{
				if ((*i)->doWork())
				{
					(*i)->callOnExit();
					i = proccess.erase(i);
					if (i == proccess.end())
					{
						break;
					}
				}
			}
		}
	private:
		std::vector<ProcessInterface*> proccess;
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
		Process(std::function<void(Process&)> onExit)
			: onExit(onExit)
			, foundSaveGame(nullptr)
			, encounterdError({})
			, workload(1)
		{
		}

		bool doWork() override
		{
			while (workload)
			{
				workload = rand() % 2;
				return false;
			}

			bool networkFailed = rand() % 2;
			if (networkFailed)
			{
				encounterdError = Error{ Error::Reason::NoNetwork, "You pleb got no network" };
				return true;
			}

			bool cloudFailed = rand() % 2;
			if (cloudFailed)
			{
				encounterdError = Error{ Error::Reason::CloudServerDown, "Could not reach cloud server at address 127.0.0.1" };
				return true;
			}

			bool localFailed = rand() % 2;
			if (localFailed)
			{
				encounterdError = Error{ Error::Reason::FileNotFound, "Could not find file..." };
				return true;
			}

			foundSaveGame = new SaveGame{ 42 };
			return true;
		}

		ResultType getResult() override
		{
			if (foundSaveGame)
			{
				return ok(foundSaveGame);
			}
			return err(encounterdError);
		}

		void callOnExit() override
		{
			onExit(*this);
		}
	private:
		std::function<void(Process&)> onExit;
		SaveGame* foundSaveGame;
		Error encounterdError;
		int workload;
	};
}

void handleGetSaveGameExit(GetSaveGame::Process& process)
{
	process.getResult().match(
		// on success
		[](GetSaveGame::SaveGame* saveGame)
		{
			std::cout << "Sucessfully got save game" << std::endl;
		},
		// on error
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

int main()
{
	srand((unsigned int)time(nullptr));
	ProcessMgr mgr;
	mgr.add(new GetSaveGame::Process(handleGetSaveGameExit));
	for (size_t i = 0; i < 10; i++)
	{
		mgr.update();
	}
}