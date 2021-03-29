/** A simple allocator which has a single piece of memory that will be used to
    give unique pointers for objects to initialise to.

    @note There is no functionality for clearing single objects from the chunk,
          only clearing the entire chunk is implemented.
*/
template<typename TypeOfCriticalSectionToUse = DummyCriticalSection>
class Allocator final : public LockableBase<TypeOfCriticalSectionToUse>
{
public:
    //==============================================================================
    /** 1 megabyte */
    static constexpr auto defaultAllocationSizeBytes = static_cast<size_t> (1024 * 1024 * 1);

    /** 4 bytes */
    static constexpr auto defaultAlignmentBytes = static_cast<size_t> (4);

    //==============================================================================
    /** Constructor

        @param sizeInBytes      The amount of bytes to allocate for use.
        @param alignmentInBytes The alignment to use when an object is allocated.
    */
    Allocator (size_t sizeInBytes = defaultAllocationSizeBytes,
               size_t alignmentInBytes = defaultAlignmentBytes) :
        base (sizeInBytes, true),
        sizeBytes (sizeInBytes),
        alignmentBytes (alignmentInBytes)
    {
        marker = base;
    }

    //==============================================================================
    /** @returns */
    size_t getAlignment() const noexcept            { return alignmentBytes; }
    /** @returns */
    intptr_t getCurrentPosition() const noexcept    { return (intptr_t) marker; }
    /** @returns */
    size_t getSize() const noexcept                 { return sizeBytes; }
    /** @returns */
    size_t getRemainingSpace() const noexcept       { return sizeBytes; }

    //==============================================================================
    /** Manually allocate a certain amount of memory

        @returns An address where an object can be initialised using placement-new,
                 or nullptr if there is not enough memory left.
    */
    void* allocate (size_t bytes) const
    {
        const typename LockableBase<TypeOfCriticalSectionToUse>::ScopedLock sl (this->lock);

        // Determine an allocation size that will align to our requested byte alignment:
        const auto remainder = static_cast<size_t> (bytes % alignmentBytes);
        auto allocationSize = bytes;

        if (remainder != 0)
            allocationSize += alignmentBytes - remainder;

        // Attempt allocating:
        if (isPositiveAndBelow (static_cast<size_t> (getCurrentPosition()) + allocationSize - getStartPosition(), sizeBytes))
        {
            auto* const newAddress = (void*) marker;
            marker += allocationSize;
            return newAddress;
        }

        Logger::writeToLog ("Error: Ran out of memory!");
        jassertfalse;
        return nullptr;
    }

    /** Manually allocate space and create an object at that address.

        @returns A new object placed within the allocator,
                 or nullptr if there wasn't enough space to put it.
    */
    template<typename Type, typename... Args>
    typename std::remove_cv<Type>::type* allocateObject (Args... args) const
    {
        using ObjectType = typename std::remove_cv<Type>::type;

        if (auto* address = allocate (sizeof (ObjectType)))
            return new (address) ObjectType (args...);

        return nullptr;
    }

    //==============================================================================
    /** @returns a newly allocated int8_t. */
    int8_t* allocateObject (int8_t value) const             { return allocateObject<int8_t> (value); }
    /** @returns a newly allocated int16_t. */
    int16_t* allocateObject (int16_t value) const           { return allocateObject<int16_t> (value); }
    /** @returns a newly allocated int32_t. */
    int32_t* allocateObject (int32_t value) const           { return allocateObject<int32_t> (value); }
    /** @returns a newly allocated int64_t. */
    int64_t* allocateObject (int64_t value) const           { return allocateObject<int64_t> (value); }
    /** @returns a newly allocated uint8_t. */
    uint8_t* allocateObject (uint8_t value) const           { return allocateObject<uint8_t> (value); }
    /** @returns a newly allocated uint16_t. */
    uint16_t* allocateObject (uint16_t value) const         { return allocateObject<uint16_t> (value); }
    /** @returns a newly allocated uint32_t. */
    uint32_t* allocateObject (uint32_t value) const         { return allocateObject<uint32_t> (value); }
    /** @returns a newly allocated uint64_t. */
    uint64_t* allocateObject (uint64_t value) const         { return allocateObject<uint64_t> (value); }
    /** @returns a newly allocated bool. */
    bool* allocateObject (bool value) const                 { return allocateObject<bool> (value); }
    /** @returns a newly allocated float. */
    float* allocateObject (float value) const               { return allocateObject<float> (value); }
    /** @returns a newly allocated double. */
    double* allocateObject (double value) const             { return allocateObject<double> (value); }
    /** @returns a newly allocated double. */
    long double* allocateObject (long double value) const   { return allocateObject<long double> (value); }

    //==============================================================================
    /** Attempt creating a copy of a specified object whilst allocating it
        within the instance of this class.

        @note This will fail at compile-time if the object passed in does not have a copy-constructor!

        @returns A copy of the object. If there is not enough memory left, nullptr is returned.
    */
    template<typename Type>
    typename std::remove_cv<Type>::type* createCopy (Type& object) const
    {
        using ObjectType = typename std::remove_cv<Type>::type;

        if (auto* const address = allocateObject<ObjectType>())
            return new (address) ObjectType (object);

        return nullptr;
    }

    /** Attempt creating a copy of a specified object whilst allocating it
        within the instance of this class.

        @note This will fail at compile-time if the object passed in does not have a copy-constructor!

        @returns A copy of the object. If there is not enough memory left, nullptr is returned.
    */
    template<typename Type>
    typename std::remove_cv<Type>::type* createCopy (Type* object) const
    {
        if (object != nullptr)
            return createCopy<Type> (*object);

        jassertfalse; //Not sure how you messed this up!
        return nullptr;
    }

    /** Attempt creating a copy of a specified object whilst allocating it
        within the instance of this class.

        @note This will fail at compile-time if the object passed in does not have a copy-move-constructor!

        @returns A copy of the object. If there is not enough memory left, nullptr is returned.
    */
    template<typename Type>
    typename std::remove_cv<Type>::type* createCopy (Type&& object) const
    {
        using ObjectType = typename std::remove_cv<Type>::type;

        if (auto* const address = allocateObject<ObjectType>())
            return new (address) ObjectType ((ObjectType) object);

        return nullptr;
    }

    //==============================================================================
    /** Resets the marker to the base memory location of the allocated memory. */
    void reset (bool clearMemory = false)
    {
        const typename LockableBase<TypeOfCriticalSectionToUse>::ScopedLock sl (this->lock);

        if (clearMemory)
            base.clear (sizeBytes);

        marker = base.getData(); // Reset the marker position to the beginning of the data set.
    }

private:
    //==============================================================================
    HeapBlock<uint8, false> base;
    mutable uint8* marker = nullptr;

    const size_t sizeBytes;
    const size_t alignmentBytes;

    //==============================================================================
    size_t getStartPosition() const noexcept { return (size_t) base.getData(); }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Allocator)
};
