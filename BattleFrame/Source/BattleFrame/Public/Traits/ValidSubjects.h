#pragma once

#include "CoreMinimal.h"
#include "SubjectHandle.h"
#include "ValidSubjects.generated.h"


USTRUCT()
struct BATTLEFRAME_API FValidSubjects
{
	GENERATED_BODY()

private:

	mutable std::atomic<bool> LockFlag{ false };

public:

	void Lock() const
	{
		while (LockFlag.exchange(true, std::memory_order_acquire));
	}

	void Unlock() const
	{
		LockFlag.store(false, std::memory_order_release);
	}

	TArray<FSolidSubjectHandle, TInlineAllocator<256>> Subjects;

	FValidSubjects() {};

	FValidSubjects(const FValidSubjects& ValidSubjects)
	{
		LockFlag.store(ValidSubjects.LockFlag.load());
		Subjects = ValidSubjects.Subjects;
	}

	FValidSubjects& operator=(const FValidSubjects& ValidSubjects)
	{
		LockFlag.store(ValidSubjects.LockFlag.load());
		Subjects = ValidSubjects.Subjects;
		return *this;
	}
};
