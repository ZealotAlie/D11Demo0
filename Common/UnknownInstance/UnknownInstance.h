#ifndef _UNKNOWN_INSTANCE_H_
#define  _UNKNOWN_INSTANCE_H_

class IUnknownInstance
{
public:
	virtual ~IUnknownInstance(){}
};

template< class T, bool AUTO_INSTANCE = false, class TInstance = T >
class UnknownInstance : public IUnknownInstance
{
public:
	UnknownInstance()
	{
		if( sm_pWarpInstance == nullptr )
		{
			sm_pWarpInstance = static_cast<T*>( this );
		}
	}
	~UnknownInstance()
	{
		if( sm_pWarpInstance == this )
		{
			sm_pWarpInstance = nullptr;
		}
	}

	static TInstance* Ptr()
	{
		return PtrWithInitialization< AUTO_INSTANCE >();
	}
private:

	template< bool DoInstance >
	static TInstance* PtrWithInitialization()
	{
		return sm_pWarpInstance? static_cast<TInstance*>( sm_pWarpInstance->GetResource() ) : nullptr;
	}

	template<> 
	static TInstance* PtrWithInitialization<true>()
	{
		if( !sm_pWarpInstance )
		{
			sm_pWarpInstance = new T( D3DApp::GetInstance()->GetDevice() );
			D3DApp::GetInstance()->AddUnknownInstance(sm_pWarpInstance);
		}
		return static_cast<TInstance*>( sm_pWarpInstance->GetResource() );
	}

	static T* sm_pWarpInstance;
};

template< class T, bool AUTO_INSTANCE /*= false*/, class TInstance /*= T */>
T* UnknownInstance<T, AUTO_INSTANCE, TInstance>::sm_pWarpInstance = nullptr;


template< class TSubClass, class TResource >
class ResourceInstance :public UnknownInstance< TSubClass, true, TResource >
{
	friend class UnknownInstance< TSubClass, true, TResource >;
public:
	ResourceInstance()
		:m_pResource(nullptr)
	{}
	virtual ~ResourceInstance()
	{
		ReleaseCOM(m_pResource);
	}

private:
	TResource* GetResource()
	{
		return m_pResource;
	}

protected:
	TResource* m_pResource;
};

#endif // ~_UNKNOWN_INSTANCE_H_