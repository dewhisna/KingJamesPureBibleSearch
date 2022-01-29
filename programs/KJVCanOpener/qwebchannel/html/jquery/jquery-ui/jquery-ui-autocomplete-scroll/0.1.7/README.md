# Scrollable jQuery UI Autocomplete

Scrollable jQuery UI Autocomplete Plugin for long items list.  
CSS `max-height`, `overflow-y` and `padding-right` can make scrollable list easy. But that is not beautiful. This plugin calculate with items and height of it.  
**See <a href="http://anseki.github.io/jquery-ui-autocomplete-scroll">DEMO</a>**

## Usage

- Load the `jquery.ui.autocomplete.scroll.min.js` script file after loading `jquery.ui.autocomplete.js` etc.
- The `maxShowItems` is added to Autocomplete options. It accept the number of items which is max height of items list.

## Example

```js
$('#txt1').autocomplete({
  maxShowItems: 5, // Make list height fit to 5 items when items are over 5.
  source: list1
});
```
