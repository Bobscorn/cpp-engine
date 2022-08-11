#pragma once

#include <AL/al.h>
#include <AL/alc.h>

#include <string>
#include <functional>

#include <list>
#include <memory>


namespace Audio
{
	inline const char *ALEnumToString(ALCenum error)
	{
		switch (error)
		{
		case AL_NO_ERROR:			return "No Error";
		case AL_INVALID_NAME:		return "Invalid Name Passed";
		case AL_INVALID_ENUM:		return "Invalid Enum Value Passed";
		case AL_INVALID_VALUE:		return "Invalid Value Passed";
		case AL_INVALID_OPERATION:	return "Invalid Operation Attempted";
		case AL_OUT_OF_MEMORY:		return "Operation Ran out of Memory";
		default:					return "Unknown Error";
		}
	}

	// Clears Error state on construction
	// Checks Error state on destruction and reports error if it exists
	struct ALErrorGuy
	{
		ALErrorGuy();
		ALErrorGuy(std::string description);
		~ALErrorGuy();

		inline ALCenum GetError() const { return m_Error; }

	protected:
		std::string m_Description;
		ALCenum m_Error = AL_NO_ERROR;
		bool m_EarlyError = false; // Ideally never true
	};

	bool TestForALError(const ALErrorGuy &guy);

	struct ALException : std::exception 
	{
		virtual const char *what() const noexcept { return "Unknown OpenAL Error occured"; }
	};

	struct DeviceException : ALException
	{
		virtual const char *what() const noexcept { return "Failed to obtain OpenAL device"; }
	};

	struct ContextException : ALException
	{
		virtual const char *what() const noexcept { return "Failed to obtain desired Context"; }
	};

	struct BufferExpection : ALException
	{
		virtual const char *what() const noexcept { return "Failed to create Buffer"; }
	};

	void Clear() noexcept;
	bool Report() noexcept;
	void ThrowReport(); // Throws if an error

	extern ALuint g_CreationID;

	// Throws on failure
	ALCdevice *CreateDevice(const ALCchar *name);
	ALCcontext *CreateContext(ALCdevice *device, const ALCint *params);

	struct Device
	{
		Device(ALCdevice *device = nullptr) : m_Device(device) {}
		~Device() { if (m_Device) { ALErrorGuy er{ "Closing Device" }; alcCloseDevice(m_Device); } }
		inline operator ALCdevice *() { return m_Device; }
	protected:
		ALCdevice *m_Device = nullptr;
	};

	struct Context
	{
		Context(ALCcontext *context = nullptr) : m_Context(context) {}
		~Context() { if (m_Context) { ALErrorGuy er{ "Closing context" }; alcMakeContextCurrent(NULL); alcDestroyContext(m_Context); } }
		inline operator ALCcontext *() { return m_Context; }
	protected:
		ALCcontext *m_Context = nullptr;
	};

	// Only 1 of these should be used at any given time
	struct Manager
	{
		Manager(const ALCchar *name = nullptr, const ALCint *params = nullptr);
		~Manager();

	protected:
		Device m_Device = nullptr;
		Context m_Context = nullptr;
	};



	struct ALSource;

	struct SourceReference
	{
		SourceReference(ALSource *guy) : Guy(guy) {}
		~SourceReference() = default;
		ALSource *Guy = nullptr;
	};

	struct ALSource
	{
		ALSource() : m_InternalName(0), m_Ref(std::make_shared<SourceReference>(this)) {}
		ALSource(ALuint name, ALuint createid) : m_InternalName(name), m_CreationID(createid), m_Ref(std::make_shared<SourceReference>(this)) {}
		ALSource(ALSource &&other) noexcept;
		~ALSource();

		ALSource &operator=(ALSource &&other) noexcept;

		ALuint Get();

		void Ensure()
		{
			if (m_CreationID != g_CreationID)
			{
				m_InternalName = 0;
				alGenSources(1, &m_InternalName);
				m_CreationID = g_CreationID;
			}
		}

		void Reset();
		void Clear();

		void Play();

		void SetPosition(float x, float y, float z);
		void SetVelocity(float x, float y, float z);

		void Loop();
		void NoLoop();
		bool IsLooping();

	protected:
		ALuint m_InternalName = 0;
		ALuint m_CreationID = 0;
	private:
		friend struct ALBufferI;
		std::shared_ptr<SourceReference> m_Ref;
	};

	ALSource CreateSauce();

	struct IBufferCreator
	{
		virtual ~IBufferCreator() {}

		virtual ALuint CreateBuffer() = 0;
	};

	struct ALBufferI
	{
		ALBufferI(std::shared_ptr<IBufferCreator> creator);
		ALBufferI(std::shared_ptr<IBufferCreator> create, ALuint name, ALuint creationid);
		ALBufferI(const ALBufferI &other);
		ALBufferI(ALBufferI &&other) noexcept;
		~ALBufferI();

		ALBufferI &operator=(const ALBufferI &other);
		ALBufferI &operator=(ALBufferI &&other);

		void Ensure();

		void Reset();

		void AttachTo(ALSource &source);

	protected:
		ALuint Get();
		void Create();

		std::shared_ptr<IBufferCreator> m_BufferCreator;
		std::list<std::weak_ptr<SourceReference>> m_Refs;
		
		ALuint m_InternalName = 0;
		ALuint m_CreationID = 0;
	};

	ALBufferI CreateBufferFile(std::string file);

	struct FileBufferCreator : IBufferCreator
	{
		FileBufferCreator(std::string filename) : m_FileName(filename) {}

		virtual ALuint CreateBuffer() override;

		void SetFileName(std::string name);
		std::string GetFileName() const;

	protected:
		std::string m_FileName;
	};
}