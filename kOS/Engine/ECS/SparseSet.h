/******************************************************************/
/*!
\file      SparseSet.h
\author    Jaz Winn Ng
\par       jazwinn.ng@digipen.edu
\date      Sept 28, 2025
\brief	   SparseSet is a data structure used in the Entity Component System (ECS) architecture
		   to efficiently store and manage components associated with entities. It provides fast access,
		   insertion, and deletion of components while maintaining a compact memory layout.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/******************************************************************/
#pragma once

#include "Config/pch.h"
#include "ECS/ECSList.h"

namespace ecs {
	class ISparseSet {
	public:
		virtual ~ISparseSet() = default;
		virtual void Delete(EntityID) = 0;
		virtual bool ContainsEntity(EntityID) = 0;
		virtual void Clear() = 0;
		virtual size_t Size() = 0;
		virtual std::pmr::vector<EntityID>& GetEntityList() = 0;
		virtual void* GetBase(EntityID) = 0;
	};

	template <typename T>
	class SparseSet : public ISparseSet {
	private:

		static constexpr size_t SPARSE_MAX_SIZE = 2048; //number of entities per page
		static constexpr size_t tombstone = std::numeric_limits<size_t>::max(); //set spare index to numerical limit

		using Sparse = std::array<size_t, SPARSE_MAX_SIZE>;

		std::pmr::vector<Sparse> m_sparsePages;
		std::pmr::vector<T> m_dense;
		std::pmr::vector<EntityID> m_denseToEntity; // 1:1 vector where dense index == Entity Index

		inline void SetDenseIndex(EntityID id, size_t index) {
			size_t page = id / SPARSE_MAX_SIZE;
			size_t sparseIndex = id % SPARSE_MAX_SIZE; // Index local to a page

			if (page >= m_sparsePages.size()) {
				m_sparsePages.resize(page + 1);
				m_sparsePages[page].fill(tombstone);
			}

			Sparse& sparse = m_sparsePages[page];

			sparse[sparseIndex] = index;
		}

		inline size_t GetDenseIndex(EntityID id) {
			size_t page = id / SPARSE_MAX_SIZE;
			size_t sparseIndex = id % SPARSE_MAX_SIZE;

			if (page < m_sparsePages.size()) {
				Sparse& sparse = m_sparsePages[page];
				return sparse[sparseIndex];
			}

			return tombstone;
		}

	public:

		SparseSet(std::pmr::memory_resource* mr = std::pmr::get_default_resource())
			: m_sparsePages(mr), // Pass 'mr' to all vectors
			m_dense(mr),
			m_denseToEntity(mr)
		{
			m_dense.reserve(1000);
			m_denseToEntity.reserve(1000);
		}

		SparseSet(
			std::allocator_arg_t,
			std::pmr::polymorphic_allocator<SparseSet<T>>,
			std::pmr::memory_resource* mr
		)
			: SparseSet(mr)
		{
		}

		T* Set(EntityID id, T obj) {
			// Overwrite existing elements


			size_t index = GetDenseIndex(id);
			if (index != tombstone) {
				m_dense[index] = obj;
				m_denseToEntity[index] = id;

				return &m_dense[index];
			}

			// New index will be the back of the dense list
			SetDenseIndex(id, m_dense.size());

			m_dense.push_back(obj);
			m_denseToEntity.push_back(id);

			return &m_dense.back();
		}


		T* Get(EntityID id) {
			size_t index = GetDenseIndex(id);
			return (index != tombstone) ? &m_dense[index] : nullptr;
		}

		const T* Get(EntityID id) const{
			size_t index = GetDenseIndex(id);
			return (index != tombstone) ? &m_dense[index] : nullptr;
		}

		void* GetBase(EntityID id) override{
			size_t index = GetDenseIndex(id);
			return (index != tombstone) ? &m_dense[index] : nullptr;
		}


		void Delete(EntityID id) override {

			size_t deletedIndex = GetDenseIndex(id);

			if (m_dense.empty() || deletedIndex == tombstone) return;

			SetDenseIndex(m_denseToEntity.back(), deletedIndex);
			SetDenseIndex(id, tombstone);

			std::swap(m_dense.back(), m_dense[deletedIndex]);
			std::swap(m_denseToEntity.back(), m_denseToEntity[deletedIndex]);

			m_dense.pop_back();
			m_denseToEntity.pop_back();
		}

		size_t Size() override {
			return m_dense.size();
		}

		std::pmr::vector<EntityID>& GetEntityList() override {
			return m_denseToEntity;
		}

		bool ContainsEntity(EntityID id) override {
			return GetDenseIndex(id) != tombstone;
		}

		void Clear() override {
			m_dense.clear();
			m_sparsePages.clear();
			m_denseToEntity.clear();
		}

		bool IsEmpty() const {
			return m_dense.empty();
		}


		const std::pmr::vector<T>& Data() const {
			return m_dense;
		}


	};
}


