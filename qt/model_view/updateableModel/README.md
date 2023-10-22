# Updateable Model

UpdateableModel is a template class that provides an easy way to create models that properly signals
updates to values. The user of the class simply has to provide an updated list of values, and the
class automatically identifies what changes have been made in terms of inserts, removals and updates,
and signals these in the right way. Signalling these changes in the right way is important for a
good user experience, as it will allow things like keeping a stable selection, keeping the visible
position, and not triggering more redraws than needed.

This class is especially useful in cases where the backend just provides the complete new data or a
signal that something has changed, but no details and no warnings before the change actually happens.

## How to use

The user of class needs to:

1. Create a class that inherits from UpdateableModel, templated on the base item class you want
   (QAbstractListModel, QAbstractTableModel) and a type T that contains the data for each item in
   the model.

2. Use some kind of container to contain the data of type T (a std::vector, a QVector, a QList, whatever).

3. Implement the normal QAIM API methods like `data`, `columnCount`, `headerData`, `rowCount`
   and if needed `flags`.

4. Add an update method that takes a container (or a pair of iterators into a data structure) with
   data items that need to be in the model after update. In that method call updateData.
   updateData needs three things:

   4.1. There needs to be a comparison method lessThan that orders items of type T

       This may take the form of T having operator< defined, or some other comparison callable you
       pass in to the updateData method.

   4.3. The data needs to be sorted using that comparison

   4.4. There needs to be a `hasChanged` method that returns a DataChanges structure containing
        information on what changed (what roles and columns) if anything.

       This may be a memberfunction of the class, or a callable passed to the updateData method.

5. updateData will then iterate through the data you pass in and the data already in the model,
   and signal all changes as efficiently as possible.

## Limitations

1. The `lessThan` function is used to uniquely identify items in the model. Item1 and Item2 are the
   same item if `(!(Item1 < Item2) && !(Item2 < Item1))`

2. The model only works on "list-type" models (possibly with multiple columns per list item).
   Tree models and models where data in different columns is not related are not supported.
