// Copyright (c) 2022 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include <mbedTLScpp/X509Cert.hpp>

#include "Exceptions.hpp"


namespace DecentEnclave
{
namespace Common
{


class CertStore;


class CertStoreCert
{
public: // static members:


	using CertBaseSharedPtr = std::shared_ptr<const mbedTLScpp::X509Cert>;


#if _cplusplus < 202002L
	template<typename _T>
	struct AtomicSharedPtr
	{
		AtomicSharedPtr() :
			m_ptr(nullptr)
		{}

		AtomicSharedPtr(const AtomicSharedPtr&) = delete;

		void operator=(const AtomicSharedPtr&) = delete;

		void operator=(std::shared_ptr<_T> desired) noexcept
		{
			std::atomic_store(&m_ptr, desired);
		}

		std::shared_ptr<_T> load() const noexcept
		{
			return std::atomic_load(&m_ptr);
		}

	private:
		std::shared_ptr<_T> m_ptr;
	}; // struct AtomicSharedPtr
#else // _cplusplus < 202002L
	template<typename _T>
	using AtomicSharedPtr = std::atomic<std::shared_ptr<_T> >;
#endif // _cplusplus < 202002L


public:


	CertStoreCert() = default;


	virtual ~CertStoreCert() = default;


	virtual CertBaseSharedPtr GetCertBase() const = 0;


	virtual const std::string& GetName() const = 0;


	bool IsRegistered(const CertStore& cs) const;


protected:


	void CheckRegistration(const CertStore& cs) const
	{
		static const bool sk_isRegistered = IsRegistered(cs);
		if (!sk_isRegistered)
		{
			throw Exception(
				"Certificate named " + GetName()+
				" must be registered to a CertStore before use"
			);
		}
	}


}; // class CertStoreCert


class CertStore
{
public: // static members:


	using CertReference = std::reference_wrapper<CertStoreCert>;

	using CertMapType = std::unordered_map<std::string, CertReference>;


	/**
	 * @brief Get the singleton instance of CertStore
	 *        There can only be one instance of CertStore
	 *
	 * @return CertStore&
	 */
	static CertStore& GetMutableInstance()
	{
		static CertStore s_inst;
		return s_inst;
	}


	static const CertStore& GetInstance()
	{
		return GetMutableInstance();
	}


public:


	virtual ~CertStore() = default;


	const CertStoreCert& operator[](const std::string& name) const
	{
		auto it = m_certMap.find(name);
		if (it == m_certMap.end())
		{
			throw Exception("CertStore - certificate name not found");
		}

		return it->second.get();
	}


	CertStoreCert& operator[](const std::string& name)
	{
		auto it = m_certMap.find(name);
		if (it == m_certMap.end())
		{
			throw Exception("CertStore - certificate name not found");
		}

		return it->second.get();
	}


	bool IsRegistered(const std::string& name) const
	{
		auto it = m_certMap.find(name);
		return (it != m_certMap.end());
	}


	template<
		typename T,
		typename std::enable_if<
			std::is_base_of<CertStoreCert, T>::value,
			int
		>::type = 0
	>
	void Register()
	{
		CertStoreCert& cert = T::BuildInstance();

		Register(cert.GetName(), cert);
	}


protected:


	void Register(const std::string& name, CertStoreCert& cert)
	{
		if (IsRegistered(name))
		{
			throw Exception("CertStore - certificate name already registered");
		}

		m_certMap.emplace(name, std::ref(cert));
	}


private:


	CertStore() :
		m_certMap()
	{}


	CertMapType m_certMap;


}; // class CertStore


inline bool CertStoreCert::IsRegistered(const CertStore& cs) const
{
	return cs.IsRegistered(GetName());
}


} // namespace Common
} // namespace DecentEnclave


#define DECENTENCLAVE_CERTSTORE_CERT(CERT_NAME, CERT_TYPE) \
class DecentCert_##CERT_NAME : \
	public ::DecentEnclave::Common::CertStoreCert \
{ \
public: \
	using Base = ::DecentEnclave::Common::CertStoreCert; \
	using CertAtSharedPtr = typename Base::AtomicSharedPtr<const CERT_TYPE>; \
	friend class ::DecentEnclave::Common::CertStore; \
	static_assert( \
		std::is_base_of<::mbedTLScpp::X509Cert, CERT_TYPE>::value, \
		#CERT_TYPE " must be a child type of mbedTLScpp::X509Cert" \
	); \
	static DecentCert_##CERT_NAME& GetInstance() \
	{ \
		BuildInstance().CheckRegistration( \
			::DecentEnclave::Common::CertStore::GetInstance() \
		); \
		return BuildInstance(); \
	} \
	static void Register() \
	{ \
		auto& kr = ::DecentEnclave::Common::CertStore::GetMutableInstance(); \
		kr.Register<DecentCert_##CERT_NAME>(); \
	} \
	static void Update(std::shared_ptr<const CERT_TYPE> cert) \
	{ \
		GetInstance().UpdateCert(cert); \
	} \
	static std::shared_ptr<const CERT_TYPE> Fetch() \
	{ \
		return GetInstance().GetCert(); \
	} \
public: \
	virtual const std::string& GetName() const override \
	{ \
		return m_name; \
	} \
	virtual std::shared_ptr<const CERT_TYPE> GetCert() const \
	{ \
		return m_cert.load(); \
	} \
	virtual typename Base::CertBaseSharedPtr GetCertBase() const override \
	{ \
		return GetCert(); \
	} \
	virtual void UpdateCert(std::shared_ptr<const CERT_TYPE> cert) \
	{ \
		m_cert = cert; \
	} \
private: \
	static DecentCert_##CERT_NAME& BuildInstance() \
	{ \
		static DecentCert_##CERT_NAME s_inst; \
		return s_inst; \
	} \
	DecentCert_##CERT_NAME() : \
		m_name(#CERT_NAME), \
		m_cert() \
	{} \
	std::string m_name; \
	CertAtSharedPtr m_cert; \
};
