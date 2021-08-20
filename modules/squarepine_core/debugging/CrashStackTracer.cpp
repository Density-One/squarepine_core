//==============================================================================
/** The global crash tracer that caches all the callsites. */
class GlobalCrashTracer final
{
public:
    GlobalCrashTracer()
    {
        entries.ensureStorageAllocated (64);
    }

    ~GlobalCrashTracer()
    {
        entries.clear(); // Explicitly calling this to avoid a possible deadlock.
    }

    void push (CrashStackTracer* c)  { const EntryList::ScopedLockType sl (entries.getLock()); entries.add (c); }
    void pop (CrashStackTracer* c)   { const EntryList::ScopedLockType sl (entries.getLock()); entries.removeFirstMatchingValue (c); }

    void dump() const
    {
        String message;
        message
            << "Stack Backtrace"
            << "--------------------------------"
            << SystemStats::getStackBacktrace()
            << newLine
            << "CrashStackTracer List"
            << "--------------------------------";

        Array<Thread::ThreadID> threads;

        {
            // NB: This is here to simply make the lock lock once, and recursively...
            const EntryList::ScopedLockType sl (entries.getLock());

            for (auto* entry : entries)
                threads.addIfNotAlreadyThere (entry->threadID);
        }

        Logger::writeToLog (message);

        if (threads.isEmpty())
        {
            Logger::writeToLog ("<none>");
            return;
        }

        threads.sort();

        message.clear();
        message.preallocateBytes (64);

        for (auto id : threads)
        {
            message
                << newLine
                << String ("Thread [xyz]")
                    .replace ("xyz", String::toHexString ((pointer_sized_int) id));

            int n = 0;

            for (int i = entries.size(); --i >= 0;)
            {
                const auto& s = *entries.getUnchecked (i);

                if (s.threadID == id)
                {
                    if (n < 10)
                        message << "0";

                    message
                        << n++
                        << "[" + s.threadName + "]: "
                        << File::createFileWithoutCheckingPath (s.file).getFileName()
                        << ":" + String (s.function)
                        << ":" + String (s.line);
                }
            }
        }

        Logger::writeToLog (message);
    }

private:
    using EntryList = Array<CrashStackTracer*, CriticalSection, 100>;
    EntryList entries;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlobalCrashTracer)
};

static GlobalCrashTracer globalCrashStack;

//==============================================================================
CrashStackTracer::CrashStackTracer (const char* f, const char* fn, int l) :
    file (f),
    function (fn),
    line (l),
    threadID (Thread::getCurrentThreadId())
{
    if (auto* t = Thread::getCurrentThread())
        threadName = t->getThreadName();

    globalCrashStack.push (this);
}

CrashStackTracer::~CrashStackTracer()
{
    globalCrashStack.pop (this);
}

void CrashStackTracer::dump (void*)
{
    Logger::writeToLog (newLine);
    Logger::writeToLog ("---------- APPLICATION CRASHED ----------");
    Logger::writeToLog (newLine);
    globalCrashStack.dump();
}
