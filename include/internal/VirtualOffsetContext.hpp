#pragma once

#include <internal/VirtualOffset.hpp>
#include <internal/VirtualOffsetReference.hpp>

namespace IMock::Internal {

/// Contains static methods used to get information about virtual table sizes
/// and offsets within them.
class VirtualOffsetContext {
    public:
        /// VirtualOffsetContext is not supposed to be instantiated since it
        /// only contains static methods.
        VirtualOffsetContext() = delete;

        /// Gets the offset in the virtual table of a provided method in the
        /// interface.
        ///
        /// @param method The method to look up.
        /// @return The virtual offset of the method.
        template <typename TInterface, typename TReturn, typename ...TArguments>
        static VirtualOffset getVirtualOffset(
            TReturn (TInterface::*method)(TArguments...)) {    
            // Cast the provided method to a reference method with the same
            // offset in a reference class.
            auto referenceMethod = reinterpret_cast<
                VirtualOffset (VirtualOffsetReference::*)()>(method);
            
            // Create a VirtualOffsetReference to call the reference method on.
            static VirtualOffsetReference virtualOffsetReference;

            // Call the reference method, whose return value is its virtual
            // table offset.
            return (virtualOffsetReference.*referenceMethod)();
        }

        /// Gets the size of the virtual table of an interface.
        ///
        /// @return The size of the virtual table of the interface.
        template <typename TInterface>
        static VirtualTableSize getVirtualTableSize() {
            // Create an interface inheriting the provided interface.
            class DerivedInterface : public TInterface {
                public:
                    /// This method will come after any other methods.
                    virtual void lastMethod() = 0;
            };

            // Get the virtual offset of the last method, which will be the size
            // of the provided interface's virtual table.
            VirtualOffset virtualTableSize = getVirtualOffset(
                &DerivedInterface::lastMethod);

            // Return the virtual table size.
            return virtualTableSize;
        }
};

}
