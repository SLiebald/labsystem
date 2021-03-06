<?php
/**
 *  labsystem.m-o-p.de -
 *                  the web based eLearning tool for practical exercises
 *  Copyright (C) 2010  Marc-Oliver Pahl
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
* This module contains some often used helper functions.
*
* @module     ../include/helper.inc
* @author     Marc-Oliver Pahl
* @copyright  Marc-Oliver Pahl 2005
* @version    1.0
*/


/**
* Returns an info arrow with describing $text as title (appears at mousevoer in most browsers)
* @param $text    The description the arrow should show.
* @param $grayed  Should the arrow be grayed?
*/
function infoArrow( $text, $grayed=false ){
  return "<img src=\"../syspix/arrow_left_9_12".( $grayed ? "_gray" : '' ).".gif\" width=\"9\" height=\"12\" border=\"0\" alt=\"&lt;\" title=\"".$text."\">";
}

/**
* Returns a checkbox input with infoArrow on its right.
* @param $name      The name the input should have for further form processing.
* @param $value     The (bitmask-)value the input should have for further form processing.
* @param $rights    The user's rights bitmask. If ($rights & $value) = $value the checkbox is checked.
* @param $disabled  Should the checkbox be disabled?
* @constant Explain_UR_ The "Explain_UR_[bitmask of the current right]" entries should be present in the language file!
*/
function rightsBox( $name, $value, $rights, $disabled=false ){
  global $lng, $usr, $pge;
  return "<input type=\"checkbox\" id=\"".$name."\" name=\"".$name."\" value=\"".$value."\" tabindex=\"".$pge->nextTab++."\"".
         ( $usr->isOfKind($value, $rights) ?  " checked=\"checked\"" : '').
         ( $disabled ? " disabled readonly" : '' ).
         " onchange='isDirty=true'><label for=\"".$name."\" class=\"labsys_mop_input_field_label\">".infoArrow( $lng->get("Explain_UR_".$value), $disabled )."</label>";
}

/**
* Returns all possible user rights with the fitting checkmarks set HORIZONTALLY.
* @param $prefix    The $prefix_[bitmask of the current right] gives the name of the input for further form processing.
* @param $rights    The user's rights bitmask. If ($rights & $value) = $value the checkbox is checked.
* @param $disabled  Should the checkbox be disabled?
* @constant MAX_USER_ROLE Used to determine the amount of Userright-Boxes.
*/
function showRightsHorizontal( $prefix, $rights, $disabled=false ){
  $ret = "";
  for ($i=1; $i<=MAX_USER_ROLE; $i=$i<<1)
    $ret .= rightsBox( $prefix."_".$i, $i, $rights, $disabled );
  return $ret;
}

/**
* Returns all possible user rights with the fitting checkmarks set VERTICALLY.
* @param $prefix    The $prefix_[bitmask of the current right] gives the name of the input for further form processing.
* @param $rights    The user's rights bitmask. If ($rights & $value) = $value the checkbox is checked.
* @param $disabled  Should the checkbox be disabled?
* @constant MAX_USER_ROLE Used to determine the number of Userright-Boxes.
*/
function showRightsVertical( $prefix, $rights, $disabled=false ){
    global $lng;
  $ret = "";
  for ($i=1; $i<=MAX_USER_ROLE; $i=$i<<1)
    $ret .= rightsBox( $prefix."_".$i, $i, $rights, $disabled )."<label for=\"".$prefix."_".$i."\" class=\"labsys_mop_input_field_label\">".$lng->get("Explain_UR_".$i)."</label><br>\n";
  return $ret;
}

/**
* This function is called to cut $text at position $where if longer.
* The special thing about that function is that it decodes html entities before counting (p.e. &nbsp; -> " ").
*
* Used in ../include/classes/Menu.inc and in the element's getPropertyRow().
*
* @param $text      The text to be eventually cut.
* @param $where     The number of characters it should be cut after.
*/
function cutHTMLText( $text, $where ){
  $decodedText = html_entity_decode( $text );                    // remove html escapes (p.e. &nbsp; -> " ")...
  if ( strlen( $decodedText ) <= $where )
    return $text;
  else
    return htmlentities ( substr( $decodedText, 0, $where-2)."..." );
}

/**
* Gets called to handle the "[HTML]" beginning tag of a string (page content etc.) appropriately.
* If the string starts with [HTML] no further processing is necessary (The browser will inerprete the following tags).
* If there is no [HTML] at the beginning of the string it gets nl2br( htmlspecialchars( string ) ); so all tags get filtered.
* Since the object comes by reference it is directly manipulated! (Doesn't work on array elements in php 4.3).
* @param &$object   The string to be parsed.
*/
function parseHTML( &$object ){
  global $pge;
  if ( (substr( $object, 0, 4 ) == '[MD]') && file_exists( '../plugins/phpMarkDown/markdown.php' ) && include_once( '../plugins/phpMarkDown/markdown.php' ) ){ //phpMarkDown http://michelf.com/projects/php-markdown/
    $object = Markdown( $pge->sanitizeKeys( substr( $object, 4 ) ) ); // sanitize necessary as __ has a meaning in MarkDown...
  }
  else if ( substr( $object, 0, 6 ) == '[HTML]' ){ // HTML
    $object = substr( $object, 6 ); // just remove [HTML]
  }else{ // assume it is plain text
//    $object = preg_replace('/ ([\w\s])/', '&nbsp;\\1', nl2br( htmlspecialchars( $object ) ) );
//    $object = nl2br( htmlspecialchars( $object ) );
    $object = str_replace(array('  ', "\t"), array('&nbsp; ', '&nbsp; &nbsp; '),
                           nl2br( htmlentities( $object ) )
              );
  }
}

function tidyHTML( &$string ){
  if ( ($GLOBALS['cfg']->get('tidyHTML') == 'TRUE' ) && // tidying is configured on via config file option
       ( substr( $string, 0, 6 ) == '[HTML]' ) ) { // this is and HTML element
    if (!isset( $GLOBALS['tidy'] )) $GLOBALS['tidy'] = new tidy();
    $string = "[HTML]\n".$GLOBALS['tidy']->repairString( substr( $string, 6 ), array('indent' => TRUE, 'show-body-only' => TRUE, 'vertical-space' => TRUE, 'wrap' => 100, 'preserve-entities' => TRUE), 'utf8' );
  }
}

// Return without modifiying HTML
function getTidiedHTML( $string ){
  tidyHTML( $string );
  return $string;
}

/**
* Difference to the previous: Doesn't directly alter the data but return it.
* Gets called to handle the "[HTML]" beginning tag of a string (page content etc.) appropriately.
* If the string starts with [HTML] no further processing is necessary (The browser will inerprete the following tags).
* If there is no [HTML] at the beginning of the string it gets nl2br( htmlspecialchars( string ) ); so all tags get filtered.
* @param  $object The string to be parsed.
* @return $string The altered $object.
*/
function returnHTML( $object ){
  parseHTML( $object );
  return $object;
}

/**
* Rremoves anything with special purpose from the $text for editing.
* @param  $text   The string to be parsed.
* @return $string The altered $text.
*/
function returnEditable( $text ){ /* removes anything with special purpose for editing */
  return str_replace( "_", "&#x005F;", htmlentities( $text ) );
}

/**
* Creates a new element of type $elementID using the [ID]1 element as template.
*
* @param $elementID     ID of the element to be created (e{p,m,i,c}).
* @return $newIdx;
*/
function createNew( $elementID ){
  $elementIDsmall = strtolower( $elementID ); // for C we want to give a capital letter but the interfaces always hav small ones
  if ( !in_array ($elementIDsmall, array('c', 'p', 'm', 'i', 'l', 's')) ) return;
  $newElement = $GLOBALS[ $elementIDsmall.'DBI' ]->getData2idx( 1 ); // load default prototype
  $newElement->history .= ': '.$elementID.' auto created';

  $newIdx = $newElement->cloneAndReturnNewIdx();
  $GLOBALS['SYSINFO'] .= $elementID.$newIdx.' auto created; ';
  return $newIdx;
}

/**
* Code from http://www.tipsntutorials.com/tips/PHP/41
*
* Removes all HTML from the given content.
*
* @param HTMLcontent The HTML string.
* @return String The content without HTML.
*/
function html2txt( $HTMLcontent ){
  $search = array ("'<script[^>]*?>.*?</script>'si",  // Strip out javascript
                   "'<[/!]*?[^<>]*?>'si",          // Strip out HTML tags
                   "'([rn])[s]+'",                // Strip out white space
                   "'&(quot|#34);'i",                // Replace HTML entities
                   "'&(amp|#38);'i",
                   "'&(lt|#60);'i",
                   "'&(gt|#62);'i",
                   "'&(nbsp|#160);'i",
                   "'&(iexcl|#161);'i",
                   "'&(cent|#162);'i",
                   "'&(pound|#163);'i",
                   "'&(copy|#169);'i",
                   "'&#(d+);'e");                    // evaluate as php

  $replace = array ("",
                   "",
                   "\1",
                   "\"",
                   "&",
                   "<",
                   ">",
                   " ",
                   chr(161),
                   chr(162),
                   chr(163),
                   chr(169),
                   "chr(\1)");

  return preg_replace($search, $replace, $HTMLcontent);
}

/**
 * Removes all dangerous characters from a filename. Used for uploaded files and directory names.
 * @param unknown $fileName To be sanitized.
 * @return mixed The sanitized file name.
 */
function sanitizeFileName( $fileName ){
  return preg_replace( '/[^\da-z.-]+/i', '_', $fileName );
}

/**
 * Generates a random name. This is used to pseudonymize names.
 * @return unknown|string
 */
$vowels = array("a", "e", "o", "u");
$consonants = array("b", "c", "d", "v", "g", "t");
function randVowel()
{
  global $vowels;
  return $vowels[array_rand($vowels, 1)];
}

function randConsonant()
{
  global $consonants;
  return $consonants[array_rand($consonants, 1)];
}
function randomName(){
  $length = rand(3,5);
  $result='';
  $lastVowel=1;
  while($length-->0){
    $lastVowel +=rand(0,1); // add some uncertainty
    if ($lastVowel>=2){
      // consonant
      $result.=randConsonant();
      $lastVowel=0;
    }else{
      $result.=randVowel();
      $lastVowel++;
    }
  }
  return ucfirst($result);
}

/**
* return a copy of an array with a certain key removed
* @param $array  the array from which the key should be removed
* @param $key    the key to remove (non-existant keys are silently ignored)
* @return        a new array without the removed key
*/
function removeFromArray($array, $key) {
  unset($array[$key]);
  return $array;
}

/**
 * Creates a copy of an array with the key of a value renamed while preserving
 * order of the array.
 * @param $array    The array to change
 * @param $value    The value of the key that should be replaced
 * @param $new_key  The new key that should be set for the given value
 * @return          A new array with the key renamed
 */
function renameKeyInArray($array, $value, $new_key) {
    //find original key for value
    $orig_key = array_search($value, $array, TRUE);
    //if the array doesn't contain the value just return it without change.
    if ($orig_key === FALSE) {
        return $array;
    }

    //get all keys of the array
    $keys = array_keys($array);
    //replace the orig_key with new_key
    $keys[array_search($orig_key, $keys, TRUE)] = $new_key;
    //combine the keys and values again
    return array_combine($keys, array_values($array));
}

/**
 * These links are used for the arrow navigation in /include/page_layout.inc at the bottom.
 * They are set by EB::prev, EB::next, and EB::TOC.
 */
$linkToPreviousPage = "#";
$linkToNextPage = "#";
$linkToTOCPage = "#";

/**
 * For preserving the user privacy, the history only stores the UID.
 * This method does the backwards resolution.
 * 
 * String $text	The text to replace.
 */
function replaceUidClearName( $text ){
	require_once( INCLUDE_DIR."/classes/DBInterfaceUser.inc" );
	global $url, $uDBI;
	if (!isset($GLOBALS['uidClearNameLookup'])){
		$GLOBALS['uidClearNameLookup'] = array(); // Array uid => clear name
		$uDBI->getAllData();
		while ($nextUser = $uDBI->getNextData()){
			$pseudonym = $url->available('pseudoNames') && !($nextUser['uid']==$_SESSION['uid']);
			$GLOBALS['uidClearNameLookup'][$nextUser['uid']] = htmlentities( ($pseudonym ? randomName() : $nextUser['foreName']).' '.($pseudonym ? randomName() : $nextUser['name'] ) ); 
		}
	}
	return str_replace ( array_keys( $GLOBALS['uidClearNameLookup'] ) , array_values( $GLOBALS['uidClearNameLookup'] ) , $text );
}
?>
