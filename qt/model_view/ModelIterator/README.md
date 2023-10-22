# Model Iterator

Sometimes, you may find yourself in a situation where your data is only available in the form of
QAbstractItemModel. Iterating over data in this model using the QAIM API is quite annoying, for
instance to find a specific record. Wouldn't it be great to just be able to use std::find or
std::find_if for that? Well, with the code in ModelIterator.h and ModelIterator.cpp you do just that!

## Contents

ModelIterator contains two iterator types for iterating over a model: FlatIterator and DepthFirstIterator.
There is a helper template class DataValueWrapper that allows directly accessing values in the model,
and finally ModelAdaptor affords an easy way to wrap a model by providing a standard begin/end method
pair returning a pair of iterators ready to use with standard algorithms or a ranged based for loop.

For details, see the documentation in ModelIterator.h

main.cpp provides some code that tests the provided functionality in ModelIterator and by doing that
presents how to use the different classes.
