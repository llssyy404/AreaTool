#pragma once

template<typename T>
class Singleton
{
public:
	static T& GetInstance()
	{
		if (nullptr == m_pkInstance)
			m_pkInstance = new T();

		return *m_pkInstance;
	}

	static void Destroy()
	{
		if (m_pkInstance) delete m_pkInstance;
	}

private:
	static T* m_pkInstance;
};

template<typename T>
T* Singleton<T>::m_pkInstance = nullptr;