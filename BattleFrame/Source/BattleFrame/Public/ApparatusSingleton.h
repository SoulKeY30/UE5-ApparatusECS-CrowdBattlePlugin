/*
 * APPARATIST
 * 
 * File: Singleton.h
 * Created: 2023-02-18 15:06:22
 * Author: Vladislav Dmitrievich Turbanov (vladislav@turbanov.ru)
 * ───────────────────────────────────────────────────────────────────
 * 
 * Community forums: https://talk.turbanov.ru
 * 
 * Copyright 2019 - 2023, SP Vladislav Dmitrievich Turbanov
 * Made in Russia, Moscow City, Chekhov City
 */

#pragma once

#include "Machine.h"


/**
 * Get the singleton subject having certain traits.
 * 
 * @tparam T The mandatory trait to have.
 * @tparam Ts The optional additional traits to have.
 * @param Mechanism The mechanism to query within.
 * @return The handle for the subject.
 */
template < typename T, typename... Ts >
inline FSubjectHandle
GetSingletonSubject(AMechanism* const Mechanism)
{
	check(Mechanism != nullptr);
	const auto Filter = FFilter::Make<T, Ts...>();
	const auto Chain = Mechanism->Enchain(Filter);

	auto Cursor = Chain->Iterate(0, 1);
	if (!Cursor.Provide())
	{
		return FSubjectHandle::Invalid;
	}

	auto SubjectHandle = Cursor.GetSubject();
	return SubjectHandle;
}

/**
 * Get the singleton trait of a subject having certain traits.
 * 
 * @tparam T The mandatory trait to have and return.
 * @tparam Ts The optional additional traits to have.
 * @param Mechanism The mechanism to query within.
 * @return A copy of the trait.
 */
template < typename T, typename... Ts >
inline T
GetSingletonTrait(AMechanism* const Mechanism)
{
	check(Mechanism != nullptr);
	const auto Filter = FFilter::Make<T, Ts...>();
	const auto Chain = Mechanism->Enchain(Filter);

	auto Cursor = Chain->Iterate(0, 1);
	if (!Cursor.Provide())
	{
		return FSubjectHandle::Invalid;
	}

	auto Trait = Cursor.template GetTrait<T>();
	return MoveTempIfPossible(Trait);
}
