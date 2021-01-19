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

		Result()
			: value(std::nullopt)
			, error(std::nullopt)
		{

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

		Process(std::function<void(ResultType)> onExit)
			: onExit(onExit)
		{
		}

		void callOnExit() override
		{
			onExit(result);
		}

	protected:
		void setResult(ResultType result)
		{
			this->result = result;
		}
	private:
		std::function<void(ResultType)> onExit;
		ResultType result;
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
		Process(std::function<void(ResultType)> onExit)
			: BaseType(onExit) { }

		bool doWork() override
		{
			while (workload)
			{
				workload = rand() % 5;
				return false;
			}

			bool networkFailed = rand() % 5;
			if (networkFailed)
			{
				setResult(err(Error{ Error::Reason::NoNetwork, "You pleb got no network" }));
				return true;
			}

			bool cloudFailed = rand() % 5;
			if (cloudFailed)
			{
				setResult(err(Error{ Error::Reason::CloudServerDown, "Could not reach cloud server at address 127.0.0.1" }));
				return true;
			}

			bool localFailed = rand() % 5;
			if (localFailed)
			{
				setResult(err(Error{ Error::Reason::FileNotFound, "Could not find file..." }));
				return true;
			}

			setResult(ok(new SaveGame{ 42 }));
			return true;
		}
	private:
		int workload = 1;
	};
}

void handleGetSaveGameExit(GetSaveGame::Process::ResultType result)
{
	result.match(
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

int main()
{
	ProcessMgr mgr;
	mgr.add(new GetSaveGame::Process(handleGetSaveGameExit));
	for (size_t i = 0; i < 10; i++)
	{
		mgr.update();
	}
}