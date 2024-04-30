#pragma once

#include "Common.hpp"
#include "Exceptions.hpp"
#include "LibInitializer.hpp"

#include "Internal/Memory.hpp"

#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{

	/**
	 * @brief Provide a pure interface class for getting the pointer to the
	 *        mbed TLS's object/context struct.
	 *
	 * @tparam _CObjType The type of the mbed TLS's object/context struct.
	 */
	template<typename _CObjType>
	class ObjIntf
	{
	public: // Static members

		using CObjType = _CObjType;

	public:
		ObjIntf() = default;

		// LCOV_EXCL_START
		virtual ~ObjIntf() = default;
		// LCOV_EXCL_STOP

		/**
		 * @brief Function for getting the pointer to the mbed TLS's
		 *        object/context struct.
		 *
		 * @return pointer to the mbed TLS's object/context struct.
		 */
		virtual const CObjType* IntfGet() const = 0;

		/**
		 * @brief Function for getting the pointer to the mbed TLS's
		 *        object/context struct.
		 *
		 * @return pointer to the mbed TLS's object/context struct.
		 */
		virtual CObjType* IntfGet() = 0;

	protected:

		/**
		 * @brief Function for getting the non-const pointer to the mbed TLS's
		 *        object/context struct, in case the child function needs to access
		 *        the non-const struct even though the cpp object is const.
		 *        NOTE: The child class is resposible for avoiding undefined behavior.
		 *
		 * @return pointer to the mbed TLS's object/context struct.
		 */
		virtual CObjType* IntfMutGet() const = 0;
	};

	/**
	 * @brief The base of normal allocators
	 *
	 */
	struct DefaultAllocBase
	{
		template<typename T, class... _Args>
		static T* NewObject(_Args&&... __args)
		{
			return Internal::NewObject<T, _Args...>(std::forward<_Args>(__args)...);
		}

		template<typename T>
		static void DelObject(T* ptr) noexcept
		{
			return Internal::DelObject(ptr);
		}
	};

	/**
	 * @brief The base allocator for a type of borrowed object
	 *
	 * @tparam _CObjType The type of the mbed TLS C object.
	 */
	template<typename _CObjType>
	struct BorrowAllocBase : public DefaultAllocBase
	{
		typedef _CObjType      CObjType;

		template<typename T, class... _Args>
		static T* NewObject(_Args&&... /* __args */)
		{
			return nullptr;
		}

		template<typename T>
		static void DelObject(T* /* ptr */) noexcept
		{}

		static void Init(CObjType* /* ptr */)
		{}

		static void Free(CObjType* /* ptr */) noexcept
		{}
	};

	/**
	 * @brief The trait template, for easier defining the trait for mbed TLS cpp object.
	 *
	 * @tparam _ObjAllocator The allocator for the mbed TLS C object.
	 * @tparam _isBorrower   Is the type a borrower?
	 * @tparam _isConst      Is the inner mbed TLS C object a constant?
	 */
	template<typename _ObjAllocator,
		bool _isBorrower,
		bool _isConst>
	struct ObjTraitBase
	{
		typedef _ObjAllocator  ObjAllocator;

		typedef typename ObjAllocator::CObjType     CObjType;

		static constexpr bool sk_isBorrower = _isBorrower;
		static constexpr bool sk_isConst    = _isConst;
	};

	/** @brief	An object base class for MbedTLS objects. */
	template<typename _ObjTrait>
	class ObjectBase : public ObjIntf<typename _ObjTrait::CObjType>
	{
	public: // Static members:

		using ObjTrait = _ObjTrait;
		using CObjType = typename ObjTrait::CObjType;
		using Allocator = typename ObjTrait::ObjAllocator;

	protected: // method will be used in constructors and destructors:

		/** @brief	Free the current object. */
		void FreeBaseObject() noexcept
		{
			constexpr bool isBorrower = ObjTrait::sk_isBorrower;
			if(!isBorrower)
			{
				if(m_ptr != nullptr)
				{
					Allocator::Free(m_ptr); //assume noexcept

					Allocator::DelObject(m_ptr); //noexcept

					m_ptr = nullptr;
				}
			}
			else
			{
				m_ptr = nullptr;
			}
		}

		/** @brief Initialize the current object. */
		void InitBaseObject()
		{
			if (m_ptr != nullptr)
			{
				FreeBaseObject();
			}
			m_ptr = Allocator::template NewObject<CObjType>();
			Allocator::Init(m_ptr);
		}

		/** @brief Swaps with the given right hand side. */
		void SwapBaseObject(ObjectBase& rhs) noexcept
		{
			std::swap(m_ptr, rhs.m_ptr);
		}

	public: // method will be used in constructors and destructors:

		/**
		 * @brief A non-virtual method for getting \c m_ptr , so that it
		 *        guarantees to return \c m_ptr (unlike \c Get() virtual method).
		 */
		CObjType* NonVirtualGet() noexcept
		{
			return m_ptr;
		}

		/**
		 * @brief A non-virtual method for getting \c m_ptr , so that it
		 *        guarantees to return \c m_ptr (unlike \c Get() virtual method).
		 */
		const CObjType* NonVirtualGet() const noexcept
		{
			return m_ptr;
		}

		/**
		 * @brief	Gets the pointer to the MbedTLS object.
		 *
		 * @exception None No exception thrown
		 * @return	The pointer to the MbedTLS object.
		 */
		const CObjType* Get() const noexcept
		{
			return m_ptr;
		}

		/**
		 * @brief	Gets the pointer to the MbedTLS object.
		 *
		 * @exception None No exception thrown
		 * @return	The pointer to the MbedTLS object.
		 */
		template<typename _dummy_ObjTrait = ObjTrait, enable_if_t<!_dummy_ObjTrait::sk_isConst, int> = 0>
		CObjType* Get() noexcept
		{
			return m_ptr;
		}

	public:

		/**
		 * @brief Construct a new mbedTLS Object Base. Usually this object base
		 *        owns the object (i.e., allocate & init at begining, free at exit).
		 *
		 * @exception Unclear may throw std::bad_alloc
		 */
		template<typename _dummy_ObjTrait = ObjTrait, enable_if_t<!_dummy_ObjTrait::sk_isBorrower, int> = 0>
		ObjectBase() :
			m_libInit(LibInitializer::GetInst()), //noexcept
			m_ptr(nullptr)
		{
			InitBaseObject();
		}

		/**
		 * @brief Construct a new mbedTLS Object Base. Usually this object base
		 *        DOES NOT own the object (i.e., no allocation, init, & free).
		 *
		 * @exception None No exception thrown
		 * @param ptr The pointer to the not null mbedTLS C object.
		 */
		template<typename _dummy_ObjTrait = ObjTrait, enable_if_t<_dummy_ObjTrait::sk_isBorrower, int> = 0>
		ObjectBase(CObjType* ptr) noexcept :
			m_libInit(LibInitializer::GetInst()), //noexcept
			m_ptr(ptr)
		{}

		ObjectBase(const ObjectBase& other) = delete;

		/**
		 * @brief	Move constructor
		 *
		 * @exception None No exception thrown
		 * @param [in,out]	other	The other instance.
		 */
		ObjectBase(ObjectBase&& rhs) noexcept :
			m_libInit(rhs.m_libInit), //noexcept
			m_ptr(rhs.m_ptr)
		{
			rhs.m_ptr = nullptr;
		}

		// LCOV_EXCL_START
		/** @brief	Destructor */
		virtual ~ObjectBase()
		{
			FreeBaseObject();
		}
		// LCOV_EXCL_STOP

		ObjectBase& operator=(const ObjectBase& other) = delete;

		/**
		 * @brief	Move assignment operator. The RHS will become empty afterwards.
		 *
		 * @param [in,out]	rhs	The right hand side.
		 *
		 * @return	A reference to this object.
		 */
		ObjectBase& operator=(ObjectBase&& rhs) noexcept
		{
			if (this != &rhs)
			{
				//Free the object to prevent memory leak.
				FreeBaseObject(); //noexcept

				m_ptr = rhs.m_ptr;

				rhs.m_ptr = nullptr;
			}
			return *this;
		}

		/**
		 * @brief	Releases the ownership of the MbedTLS Object, and
		 * 			return the pointer to the MbedTLS object.
		 *
		 * @exception None No exception thrown
		 * @return	The pointer to the MbedTLS object.
		 */
		template<typename _dummy_ObjTrait = ObjTrait, enable_if_t<!_dummy_ObjTrait::sk_isConst, int> = 0>
		CObjType* Release() noexcept
		{
			CObjType* tmp = m_ptr;

			m_ptr = nullptr;

			return tmp;
		}

		/**
		 * @brief	Query if this is the actual owner of MbedTLS object.
		 *
		 * @exception None No exception thrown
		 * @return	True if it's, false if not.
		 */
		virtual bool IsOwner() const noexcept
		{
			return ObjTrait::sk_isBorrower;
		}

		/**
		 * @brief	Query if c object held by this object is null
		 *
		 * @exception None No exception thrown
		 * @return	True if null, false if not.
		 */
		virtual bool IsNull() const noexcept
		{
			return m_ptr == nullptr;
		}

		/**
		 * @brief Implementation for \ref mbedTLScpp::ObjIntf::IntfGet() "ObjIntf::IntfGet()" method.
		 *
		 * @return const CObjType*
		 */
		virtual const CObjType* IntfGet() const override
		{
			return m_ptr;
		}

		/**
		 * @brief Implementation for \ref mbedTLScpp::ObjIntf::IntfGet() "ObjIntf::IntfGet()" method.
		 *
		 * @exception RuntimeException Thrown when the inner mbed TLS C object is const.
		 *
		 * @return CObjType*
		 */
		virtual CObjType* IntfGet() override
		{
			if (ObjTrait::sk_isConst)
			{
				throw RuntimeException("ObjectBase::IntfGet - A const object can't return non-const C object pointer.");
			}
			return m_ptr;
		}

	protected:

		/**
		 * @brief	Swaps with the given right hand side.
		 *
		 * @exception None No exception thrown
		 * @param	rhs	The right hand side.
		 */
		virtual void Swap(ObjectBase& rhs) noexcept
		{
			SwapBaseObject(rhs);
		}

		/**
		 * @brief Set the pointer to the mbedTLS.
		 *
		 * @exception None No exception thrown
		 * @param ptr pointer to the mbedTLS
		 */
		void SetPtr(CObjType* ptr) noexcept
		{
			m_ptr = ptr;
		}

		/**
		 * @brief Check if the current instance is holding a null pointer for
		 *        the mbedTLS object. If so, exception will be thrown. Helper
		 *        function to be called before accessing the mbedTLS object.
		 *        NOTE: this function should called&override by the child class
		 *        to receive the child class's name.
		 *
		 * @exception InvalidObjectException Thrown when the current instance is
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @param objTypeName The name of the child class that inherit this base
		 *                    class.
		 */
		virtual void NullCheck(const std::string& objTypeName) const
		{
			if (IsNull())
			{
				throw InvalidObjectException(objTypeName);
			}
		}

		/**
		 * @brief In case the child class has const function that needs to access
		 *        the C pointer in non-const way.
		 *        NOTE: The child class is resposible for avoiding undefined behavior.
		 *
		 * @exception None No exception thrown
		 * @return CObjType* The pointer to the MbedTLS object.
		 */
		CObjType* MutableGet() const noexcept
		{
			return m_ptr;
		}

		/**
		 * @brief Implementation for \ref mbedTLScpp::ObjIntf::IntfMutGet() "ObjIntf::IntfMutGet()" method.
		 *
		 * @return CObjType*
		 */
		virtual CObjType* IntfMutGet() const override
		{
			return m_ptr;
		}

	private:
		LibInitializer& m_libInit;
		CObjType * m_ptr; // NOTE: must be a pointer.
	};

	namespace Internal
	{
		template <typename DerivedCppObjType, typename MbedTlsCType>
		struct IsCppObjOfCtypeImpl
		{
			using CleanDerivedCppObjType = typename remove_cvref<DerivedCppObjType>::type;

			template<typename _ObjTrait>
			static constexpr std::is_same<typename ObjectBase<_ObjTrait>::CObjType, MbedTlsCType>
				test(const ObjectBase<_ObjTrait> *);

			static constexpr std::false_type test(...);

			using type = decltype(test(std::declval<CleanDerivedCppObjType*>()));
		};
	}

	/**
	 * @brief Checking if a given class type is derived from the \c ObjectBase
	 *        class, and if so, checking if its internal mbed TLS C type matches
	 *        the given type.
	 *        It's based on idea from https://stackoverflow.com/questions/
	 *        34672441/stdis-base-of-for-template-classes
	 *
	 * @tparam DerivedCppObjType  The mbed TLS cpp type to check with.
	 * @tparam MbedTlsCType       The mbed TLS C type to check with.
	 */
	template <typename DerivedCppObjType, typename MbedTlsCType>
	using IsCppObjOfCtype = typename Internal::IsCppObjOfCtypeImpl<DerivedCppObjType, MbedTlsCType>::type;

	static_assert(IsCppObjOfCtype<std::string, bool>::value == false, "Programming Error");
}
