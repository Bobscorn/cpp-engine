#include "Audio.h"

#include "Helpers/DebugHelper.h"

#ifdef __linux__
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif

#include <cstring>

#define AUDIO_SOURCE_CHECK if ((!alcGetCurrentContext()) || (!alIsSource(m_InternalName))) return;
#define AUDIO_BUFFER_CHECK if ((!alcGetCurrentContext()) || (!alIsBuffer(m_InternalName))) return;

namespace Audio
{
	ALuint g_CreationID = 0;

	ALuint FileBufferCreator::CreateBuffer()
	{
		ALuint outbuf = 0;
		Uint8 *buf = 0;
		Uint32 len = 0;
		SDL_AudioSpec spec;
		if (SDL_LoadWAV(m_FileName.c_str(), &spec, &buf, &len) == NULL)
		{
			return 0;
		}

		ALenum format;
		switch (spec.format)
		{
		case AUDIO_S8:
		case AUDIO_U8:
			format = spec.channels == 2 ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8;
			break;
		case AUDIO_U16:
		case AUDIO_S16:
			format = spec.channels == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
			break;
		case AUDIO_S32LSB:
		case AUDIO_S32MSB:
		case AUDIO_F32LSB:
		case AUDIO_F32MSB:
			format = spec.channels == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
			{
				SDL_AudioCVT cvt;
				SDL_BuildAudioCVT(&cvt, spec.format, spec.channels, spec.freq, AUDIO_S16, spec.channels == 2 ? 2 : 1, 48000);
				cvt.len = len;
				cvt.buf = (Uint8*)SDL_malloc((size_t)(cvt.len * cvt.len_mult));
				std::memcpy(cvt.buf, buf, len);
				SDL_ConvertAudio(&cvt);

				Clear();
				alGenBuffers(1, &outbuf);
				alBufferData(outbuf, format, cvt.buf, cvt.len_cvt, spec.freq);
				SDL_free((void *)cvt.buf);
				SDL_FreeWAV(buf);
				ThrowReport();

				return outbuf;
			}
			break;
		default:
			SDL_FreeWAV(buf);
			return 0;
		}

		Clear();
		alGenBuffers(1, &outbuf);
		alBufferData(outbuf, format, buf, len, spec.freq);
		SDL_FreeWAV(buf);
		ThrowReport();
		
		return outbuf;
	}

	void FileBufferCreator::SetFileName(std::string name)
	{
		m_FileName = name;
	}

	std::string FileBufferCreator::GetFileName() const
	{
		return m_FileName;
	}

	Manager::Manager(const ALCchar *name, const ALCint *params) : m_Device(CreateDevice(name)), m_Context(CreateContext(m_Device, params))
	{
		if (!alcMakeContextCurrent(m_Context))
		{
			DERROR("Could not make context current");
			throw ContextException();
		}

		if (Audio::TestForALError(Audio::ALErrorGuy{ "Creating Context" }))
			throw ContextException();

		++g_CreationID;
	}
	Manager::~Manager()
	{
	}
}

Audio::ALErrorGuy::ALErrorGuy() : ALErrorGuy("Unknown")
{
}

Audio::ALErrorGuy::ALErrorGuy(std::string description) : m_Description(description), m_Error(AL_NO_ERROR)
{
	if (alcGetCurrentContext())
	{
		m_Error = alGetError();
		if (m_Error != AL_NO_ERROR)
			m_EarlyError = true;
	}
	else
	{
		DINFO("No context to check for errors with");
	}
}

Audio::ALErrorGuy::~ALErrorGuy()
{
	if (alcGetCurrentContext())
	{
		m_Error = alGetError();
		if (m_Error != AL_NO_ERROR)
		{
			DERROR(ALEnumToString(m_Error) + ": " + m_Description);
		}
	}
}

bool Audio::TestForALError(ALErrorGuy && guy)
{
	return guy.m_Error != AL_NO_ERROR;
}

void Audio::Clear() noexcept
{
	if (alcGetCurrentContext())
	{
		ALCenum er = alGetError();
		if (er != AL_NO_ERROR)
			DWARNING("Uncleared Error found for OpenAL");
	}
}

bool Audio::Report() noexcept
{
	return TestForALError(ALErrorGuy());
}

void Audio::ThrowReport()
{
	if (alcGetCurrentContext())
	{
		ALCenum er = alGetError();
		if (er != AL_NO_ERROR)
		{
			throw ALException();
		}
	}
}

ALCdevice *Audio::CreateDevice(const ALCchar *name)
{
	ALCdevice *out = nullptr;
	out = alcOpenDevice(name);
	if (!out)
		throw DeviceException();
	return out;
}

ALCcontext *Audio::CreateContext(ALCdevice *device, const ALCint *params)
{
	ALCcontext *out = alcCreateContext(device, params);
	if (!out)
	{
		Clear();
		throw ContextException();
	}
	return out;
}

Audio::ALSource Audio::CreateSauce()
{
	ALuint name = 0;
	if (g_CreationID)
		alGenSources(1, &name);
	return ALSource(name, g_CreationID);
}

Audio::ALBufferI Audio::CreateBufferFile(std::string file)
{
	std::shared_ptr<Audio::IBufferCreator> creator = std::make_shared<Audio::FileBufferCreator>(file);

	return ALBufferI{ creator };
}

Audio::ALSource::ALSource(ALSource &&other) noexcept : m_InternalName(other.m_InternalName), m_CreationID(other.m_CreationID), m_Ref(other.m_Ref)
{
	other.m_CreationID = other.m_InternalName = 0;
	other.m_Ref = nullptr;
}

Audio::ALSource::~ALSource()
{
	Reset();
}

Audio::ALSource &Audio::ALSource::operator=(ALSource &&other) noexcept
{
	Reset();

	this->m_CreationID = other.m_CreationID;
	this->m_InternalName = other.m_InternalName;
	this->m_Ref = other.m_Ref;
	other.m_CreationID = other.m_InternalName = 0;
	other.m_Ref = nullptr;

	return *this;
}

ALuint Audio::ALSource::Get()
{
	Ensure();
	return m_InternalName;
}

void Audio::ALSource::Reset()
{
	if (!alcGetCurrentContext()) return;

	if (m_CreationID == g_CreationID)
	{
		if (alIsSource(m_InternalName))
		{
			alSourceStop(m_InternalName);
			alSourcei(m_InternalName, AL_BUFFER, 0);
			alDeleteSources(1, &m_InternalName);
		}
		m_InternalName = m_CreationID = 0;
	}
	else
	{
		m_InternalName = m_CreationID = 0;
	}
}

void Audio::ALSource::Clear()
{
	AUDIO_SOURCE_CHECK;
	alSourceStop(m_InternalName);
	alSourcei(m_InternalName, AL_BUFFER, 0);
	m_Ref = std::make_shared<SourceReference>(this);
}

void Audio::ALSource::Play()
{
	if (!alcGetCurrentContext()) return;
	Ensure();
	if (alIsSource(m_InternalName))
	{
		alSourcePlay(m_InternalName);
	}
}

void Audio::ALSource::SetPosition(float x, float y, float z)
{
	if (!alcGetCurrentContext()) return;
	Ensure();
	if (alIsSource(m_InternalName))
	{
		alSource3f(m_InternalName, AL_POSITION, x, y, z);
	}
}

void Audio::ALSource::SetVelocity(float x, float y, float z)
{
	if (!alcGetCurrentContext()) return;
	Ensure();
	if (alIsSource(m_InternalName))
	{
		alSource3f(m_InternalName, AL_VELOCITY, x, y, z);
	}
}

void Audio::ALSource::Loop()
{
	if (!alcGetCurrentContext()) return;
	Ensure();
	if (alIsSource(m_InternalName))
	{
		alSourcei(m_InternalName, AL_LOOPING, AL_TRUE);
	}
}

void Audio::ALSource::NoLoop()
{
	if (!alcGetCurrentContext()) return;
	Ensure();
	if (alIsSource(m_InternalName))
	{
		alSourcei(m_InternalName, AL_LOOPING, AL_FALSE);
	}
}

bool Audio::ALSource::IsLooping()
{
	if (!alcGetCurrentContext()) return false;
	Ensure();
	if (alIsSource(m_InternalName))
	{
		int val = AL_FALSE;
		alGetSourcei(m_InternalName, AL_LOOPING, &val);
		return val;
	}
	return false;
}

Audio::ALBufferI::ALBufferI(std::shared_ptr<IBufferCreator> creator) : m_BufferCreator(creator)
{
}

Audio::ALBufferI::ALBufferI(std::shared_ptr<IBufferCreator> create, ALuint name, ALuint creationid) : m_BufferCreator(create), m_InternalName(name), m_CreationID(creationid)
{
}

Audio::ALBufferI::ALBufferI(const ALBufferI &other) : m_BufferCreator(other.m_BufferCreator)
{
}

Audio::ALBufferI::ALBufferI(ALBufferI &&other) noexcept
{
	this->m_BufferCreator = other.m_BufferCreator;
	other.m_BufferCreator = nullptr;
	
	this->m_CreationID = other.m_CreationID;
	other.m_CreationID = 0;
	
	this->m_InternalName = other.m_InternalName;
	other.m_InternalName = 0;
}

Audio::ALBufferI::~ALBufferI()
{
	Reset();
}

Audio::ALBufferI &Audio::ALBufferI::operator=(const ALBufferI &other)
{
	Reset();

	this->m_BufferCreator = other.m_BufferCreator;

	Create();

	return *this;
}

Audio::ALBufferI &Audio::ALBufferI::operator=(ALBufferI &&other)
{
	Reset();

	this->m_BufferCreator = other.m_BufferCreator;
	other.m_BufferCreator = nullptr;

	this->m_CreationID = other.m_CreationID;
	other.m_CreationID = 0;

	this->m_InternalName = other.m_InternalName;
	other.m_InternalName = 0;

	return *this;
}

void Audio::ALBufferI::Ensure()
{
	if (g_CreationID && m_CreationID != g_CreationID || m_InternalName == 0)
	{
		m_InternalName = 0;
		Create();
	}
}

ALuint Audio::ALBufferI::Get()
{
	Ensure();
	return m_InternalName;
}

void Audio::ALBufferI::Reset()
{
	if (m_CreationID == g_CreationID)
	{
		for (auto &a : m_Refs)
		{
			auto p = a.lock();
			if (p && p->Guy)
			{
				p->Guy->Clear();
			}
		}
		if (alIsBuffer(m_InternalName))
			alDeleteBuffers(1, &m_InternalName);
		m_InternalName = 0;
		m_CreationID = 0;
	}
	else
	{
		m_InternalName = 0;
	}
}

void Audio::ALBufferI::AttachTo(ALSource &source)
{
	source.Clear();
	if (alIsBuffer(m_InternalName))
	{
		alSourcei(source.Get(), AL_BUFFER, Get());
		this->m_Refs.push_back(source.m_Ref);
	}
}

void Audio::ALBufferI::Create()
{
	if (m_BufferCreator && g_CreationID)
	{
		m_InternalName = m_BufferCreator->CreateBuffer();
		m_CreationID = g_CreationID;
	}
}
