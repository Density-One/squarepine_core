#if SQUAREPINE_COMPILE_UNIT_TESTS

class BackgroundCallerTests final : public UnitTest
{
public:
    BackgroundCallerTests() : UnitTest ("BackgroundCaller", UnitTestCategories::containers)
    {
    }

    void runTest() override
    {
        std::function<void (void)> testFunction = []()
                          { std::cout << "Hello from the background thread!" << std::endl; };

        // Create a BackgroundCaller object
        auto backgroundCaller = new BackgroundCaller(testFunction);
        auto asyncUpdater = dynamic_cast<AsyncUpdater*>(backgroundCaller);

        // Trigger an async update
        asyncUpdater->triggerAsyncUpdate();

        // Simulate the message thread
        asyncUpdater->handleAsyncUpdate();

        BackgroundCaller caller(testFunction);
    }
};

#endif
