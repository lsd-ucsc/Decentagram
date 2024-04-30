#pragma once

#include "MsgDigestBase.hpp"

#include "Container.hpp"
#include "Exceptions.hpp"

#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{
	/**
	 * @brief The container type used to store the hash result (for a known hash type).
	 *
	 * @tparam _HashT The type of the hash.
	 */
	template<HashType _HashT>
	struct Hash
	{
		static constexpr HashType sk_type = _HashT;
		static constexpr size_t   sk_size = GetHashByteSize(sk_type);

		using ContainerType = std::array<uint8_t, sk_size>;

		ContainerType m_data;

		Hash() = default;

		uint8_t* data() { return m_data.data(); }
		const uint8_t* data() const { return m_data.data(); }

		typename ContainerType::size_type size() const { return m_data.size(); }

	};
	static_assert(std::is_trivial<Hash<HashType::SHA256> >::value
				== std::is_trivial<typename Hash<HashType::SHA256>::ContainerType>::value,
				"Programming Error");

	/**
	 * @brief A helper function to get the container reference. Other partial
	 *        reference functions are not provided, since they are meaningless to hashes.
	 *
	 * @tparam _HashT The type of the hash.
	 * @param hash The hash
	 * @return The container reference
	 */
	template<HashType _HashT>
	inline ContCtnReadOnlyStRef<typename Hash<_HashT>::ContainerType, Hash<_HashT>::sk_size>
		CtnFullR(const Hash<_HashT>& hash) noexcept
	{
		return CtnFullR(hash.m_data);
	}

	/**
	 * @brief The base class for Hash calculator. It can accept some raw pointer
	 *        parameters, and hash type can be specified at runtime.
	 *
	 */
	class HasherBase : public MsgDigestBase<>
	{
	public:

		HasherBase() = delete;

		/**
		 * @brief	Constructor. mbedtls_md_starts is called here.
		 *
		 * @exception mbedTLSRuntimeError  Thrown when mbed TLS C function call failed.
		 * @exception std::bad_alloc       Thrown when memory allocation failed.
		 * @param	mdInfo	The md info provided by mbed TLS library.
		 */
		HasherBase(const mbedtls_md_info_t& mdInfo)  :
			MsgDigestBase(mdInfo, false)
		{
			MBEDTLSCPP_MAKE_C_FUNC_CALL(HasherBase::HasherBase, mbedtls_md_starts, NonVirtualGet());
		}

		/**
		 * @brief Move Constructor. The `rhs` will be empty/null afterwards.
		 *
		 * @param rhs The other HasherBase instance.
		 */
		HasherBase(HasherBase&& rhs) noexcept :
			MsgDigestBase(std::forward<MsgDigestBase>(rhs))
		{}

		HasherBase(const HasherBase& rhs) = delete;

		// LCOV_EXCL_START
		/** @brief	Destructor */
		virtual ~HasherBase() = default;
		// LCOV_EXCL_STOP

		/**
		 * @brief Move assignment. The `rhs` will be empty/null afterwards.
		 *
		 * @param rhs The other HasherBase instance.
		 * @return HasherBase& A reference to this instance.
		 */
		HasherBase& operator=(HasherBase&& rhs) noexcept
		{
			MsgDigestBase::operator=(std::forward<MsgDigestBase>(rhs));

			return *this;
		}

		HasherBase& operator=(const HasherBase& other) = delete;

		/**
		 * @brief Updates the calculation with the given data.
		 *
		 *
		 * @exception InvalidObjectException Thrown when the current instance is
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 * @tparam ContainerType The type of the container that stores the data.
		 * @param data The data to be hashed.
		 */
		template<typename ContainerType, bool ContainerSecrecy>
		void Update(const ContCtnReadOnlyRef<ContainerType, ContainerSecrecy>& data)
		{
			NullCheck();

			MBEDTLSCPP_MAKE_C_FUNC_CALL(HasherBase::Update, mbedtls_md_update,
				Get(),
				static_cast<const unsigned char*>(data.BeginPtr()),
				data.GetRegionSize());
		}

		/**
		 * @brief Finishes the hash calculation and get the hash result.
		 *
		 * @exception InvalidObjectException Thrown when the current instance is
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 * @return std::vector<uint8_t> The hash result.
		 */
		std::vector<uint8_t> Finish()
		{
			NullCheck();

			const mbedtls_md_info_t* mdInfo = mbedtls_md_info_from_ctx(Get());
			const size_t size = mbedtls_md_get_size(mdInfo);
			if (size == 0)
			{
				throw UnexpectedErrorException("HasherBase is not null, but mbedtls_md_get_size returns zero.");
			}

			std::vector<uint8_t> hash(size);

			MBEDTLSCPP_MAKE_C_FUNC_CALL(HasherBase::Finish, mbedtls_md_finish,
				Get(),
				static_cast<unsigned char*>(hash.data()));

			return hash;
		}

		/**
		 * @brief Restart the hash calculation, so that the previous hash state
		 *        will be wiped out. It's useful if you want to reuse the same
		 *        hasher instance.
		 *
		 * @exception InvalidObjectException Thrown when the current instance is
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 */
		void Restart()
		{
			NullCheck();

			MBEDTLSCPP_MAKE_C_FUNC_CALL(HasherBase::Restart, mbedtls_md_starts, Get());
		}

	protected:

		void UpdateNoCheck(const void* data, size_t size)
		{
			MBEDTLSCPP_MAKE_C_FUNC_CALL(HasherBase::UpdateNoCheck, mbedtls_md_update,
				Get(),
				static_cast<const unsigned char*>(data),
				size);
		}
	};

	/**
	 * @brief The hash calculator. Only accept C++ objects as parameters, and
	 *        hash type must be specified at compile time.
	 *
	 * @tparam _HashTypeValue
	 */
	template<HashType _HashTypeValue>
	class Hasher : public HasherBase
	{
	public: //static members:
		static constexpr size_t sk_hashByteSize = GetHashByteSize(_HashTypeValue);

	public:

		/**
		 * @brief Construct a new Hasher object
		 *
		 * @exception mbedTLSRuntimeError  Thrown when mbed TLS C function call failed.
		 * @exception std::bad_alloc       Thrown when memory allocation failed.
		 */
		Hasher() :
			HasherBase(GetMdInfo(_HashTypeValue))
		{}

		// LCOV_EXCL_START
		/**
		 * @brief Destroy the Hasher object
		 *
		 */
		virtual ~Hasher() = default;
		// LCOV_EXCL_STOP

		/**
		 * @brief Move Constructor. The `rhs` will be empty/null afterwards.
		 *
		 * @param rhs The other Hasher instance.
		 */
		Hasher(Hasher&& rhs) noexcept :
			HasherBase(std::forward<HasherBase>(rhs)) //noexcept
		{}

		Hasher(const Hasher& rhs) = delete;

		/**
		 * @brief Move assignment. The `rhs` will be empty/null afterwards.
		 *
		 * @param rhs The other Hasher instance.
		 * @return Hasher& A reference to this instance.
		 */
		Hasher& operator=(Hasher&& rhs) noexcept
		{
			HasherBase::operator=(std::forward<HasherBase>(rhs)); //noexcept

			return *this;
		}

		Hasher& operator=(const Hasher& other) = delete;

		/**
		 * @brief Finishes the hash calculation and get the hash result.
		 *
		 * @exception InvalidObjectException Thrown when the current instance is
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 * @return Hash<_HashTypeValue> The hash result.
		 */
		Hash<_HashTypeValue> Finish()
		{
			NullCheck();

			return FinishNoCheck();
		}

		/**
		 * @brief Update the hash calculation with a list of Input Data Items.
		 *        NOTE: This function will not clean the previous state, thus,
		 *        it will update the calculation state based on the existing state;
		 *        Thus, you may need to call restart first.
		 *
		 * @exception InvalidObjectException Thrown when the current instance is
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 * @tparam ListLen The length of the list.
		 * @param list The list of Input Data Items.
		 * @return Hash<_HashTypeValue> The hash result.
		 */
		template<size_t ListLen>
		Hash<_HashTypeValue> CalcList(const InDataList<ListLen>& list)
		{
			NullCheck();

			for(auto it = list.begin(); it != list.end(); ++it)
			{
				UpdateNoCheck(it->m_data, it->m_size);
			}

			return FinishNoCheck();
		}

		/**
		 * @brief Update the hash calculation with a sequence of containers wrapped
		 *        by ContCtnReadOnlyRef. The sequence of containers can be in any
		 *        length.
		 *        NOTE: This function will not clean the previous state, thus,
		 *        it will update the calculation state based on the existing state;
		 *        Thus, you may need to call restart first.
		 *
		 * @exception InvalidObjectException Thrown when the current instance is
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 * @tparam Args The type of the container wrapped by ContCtnReadOnlyRef
		 * @param args The container.
		 * @return Hash<_HashTypeValue> The hash result.
		 */
		template<class... Args>
		Hash<_HashTypeValue> Calc(Args... args)
		{
			return CalcList(ConstructInDataList(args...));
		}

	private:

		Hash<_HashTypeValue> FinishNoCheck()
		{
			Hash<_HashTypeValue> hash;

			MBEDTLSCPP_MAKE_C_FUNC_CALL(Hasher::FinishNoCheck, mbedtls_md_finish,
				Get(),
				static_cast<unsigned char*>(hash.m_data.data()));

			return hash;
		}
	};
}
