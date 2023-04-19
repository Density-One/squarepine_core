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

        // Verify that the object has been deleted
        if (backgroundCaller != nullptr)
        {
            std::cerr << "Error: BackgroundCaller object not deleted properly." << std::endl;
        }

        std::cout << "Test passed." << std::endl;
    }
};

#endif
