#pragma once

#include <internal/VirtualTableOffset.hpp>
#include <internal/VirtualTableOffsetReference.hpp>
#include <Method.hpp>

namespace IMock {
namespace Internal {

/// Contains static methods used to get information about virtual table sizes
/// and offsets within them.
class VirtualTableOffsetContext {
    private:
        /// An interface inheriting the provided interface.
        template <typename TInterface>
        class DerivedInterface : public TInterface {
            public:
                /// This method will come after any other methods.
                virtual void lastMethod() = 0;
        };

    public:
        /// VirtualTableOffsetContext is not supposed to be instantiated since
        /// it only contains static methods.
        VirtualTableOffsetContext() = delete;

        /// Gets the offset in the virtual table of a provided method in the
        /// interface.
        ///
        /// @param method The method to look up.
        /// @return The virtual table offset of the method.
        template <typename TInterface, typename TReturn, typename ...TArguments>
        static VirtualTableOffset getVirtualTableOffset(
            Method<TInterface, TReturn, TArguments...> method) {
            // Declare a using for reference methods.
            using ReferenceMethod
                = Method<VirtualTableOffsetReference, VirtualTableOffset>;

            // Cast the provided method to a reference method with the same
            // offset in a reference class.
            ReferenceMethod referenceMethod
                = reinterpret_cast<ReferenceMethod>(method);
            
            // Create a VirtualTableOffsetReference to call the reference method
            // on.
            static VirtualTableOffsetReference virtualTableOffsetReference;

            // Call the reference method, whose return value is its virtual
            // table offset.
            return (virtualTableOffsetReference.*referenceMethod)();
        }

        /// Gets the size of the virtual table of an interface.
        ///
        /// @return The size of the virtual table of the interface.
        template <typename TInterface>
        static VirtualTableSize getVirtualTableSize() {
            // Get the virtual table offset of the last method, which will be
            // the size of the provided interface's virtual table.
            VirtualTableOffset virtualTableSize = getVirtualTableOffset(
                &DerivedInterface<TInterface>::lastMethod);

            // Return the virtual table size.
            return virtualTableSize;
        }
};

}
}
