/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

"use strict";

this.EXPORTED_SYMBOLS = ["ShortcutUtils"];

ChromeUtils.import("resource://gre/modules/Services.jsm");
ChromeUtils.import("resource://gre/modules/XPCOMUtils.jsm");

XPCOMUtils.defineLazyGetter(this, "PlatformKeys", function() {
  return Services.strings.createBundle(
    "chrome://global-platform/locale/platformKeys.properties");
});

XPCOMUtils.defineLazyGetter(this, "Keys", function() {
  return Services.strings.createBundle(
    "chrome://global/locale/keys.properties");
});

var ShortcutUtils = {
  /**
    * Prettifies the modifier keys for an element.
    *
    * @param Node aElemKey
    *        The key element to get the modifiers from.
    * @param boolean aNoCloverLeaf
    *        Pass true to use a descriptive string instead of the cloverleaf symbol. (OS X only)
    * @return string
    *         A prettified and properly separated modifier keys string.
    */
  prettifyShortcut(aElemKey, aNoCloverLeaf) {
    let elemString = "";
    let elemMod = aElemKey.getAttribute("modifiers");
    let haveCloverLeaf = false;

    if (elemMod.match("accel")) {
      if (Services.appinfo.OS == "Darwin") {
        // XXX bug 779642 Use "Cmd-" literal vs. cloverleaf meta-key until
        // Orion adds variable height lines.
        if (aNoCloverLeaf) {
          elemString += "Cmd-";
        } else {
          haveCloverLeaf = true;
        }
      } else {
        elemString += PlatformKeys.GetStringFromName("VK_CONTROL") +
          PlatformKeys.GetStringFromName("MODIFIER_SEPARATOR");
      }
    }
    if (elemMod.match("access")) {
      if (Services.appinfo.OS == "Darwin") {
        elemString += PlatformKeys.GetStringFromName("VK_CONTROL") +
          PlatformKeys.GetStringFromName("MODIFIER_SEPARATOR");
      } else {
        elemString += PlatformKeys.GetStringFromName("VK_ALT") +
          PlatformKeys.GetStringFromName("MODIFIER_SEPARATOR");
      }
    }
    if (elemMod.match("os")) {
      elemString += PlatformKeys.GetStringFromName("VK_WIN") +
        PlatformKeys.GetStringFromName("MODIFIER_SEPARATOR");
    }
    if (elemMod.match("shift")) {
      elemString += PlatformKeys.GetStringFromName("VK_SHIFT") +
        PlatformKeys.GetStringFromName("MODIFIER_SEPARATOR");
    }
    if (elemMod.match("alt")) {
      elemString += PlatformKeys.GetStringFromName("VK_ALT") +
        PlatformKeys.GetStringFromName("MODIFIER_SEPARATOR");
    }
    if (elemMod.match("ctrl") || elemMod.match("control")) {
      elemString += PlatformKeys.GetStringFromName("VK_CONTROL") +
        PlatformKeys.GetStringFromName("MODIFIER_SEPARATOR");
    }
    if (elemMod.match("meta")) {
      elemString += PlatformKeys.GetStringFromName("VK_META") +
        PlatformKeys.GetStringFromName("MODIFIER_SEPARATOR");
    }

    if (haveCloverLeaf) {
      elemString += PlatformKeys.GetStringFromName("VK_META") +
        PlatformKeys.GetStringFromName("MODIFIER_SEPARATOR");
    }

    let key;
    let keyCode = aElemKey.getAttribute("keycode");
    if (keyCode) {
      keyCode = keyCode.toUpperCase();
      try {
        let bundle = keyCode == "VK_RETURN" ? PlatformKeys : Keys;
        // Some keys might not exist in the locale file, which will throw.
        key = bundle.GetStringFromName(keyCode);
      } catch (ex) {
        Cu.reportError("Error finding " + keyCode + ": " + ex);
        key = keyCode.replace(/^VK_/, "");
      }
    } else {
      key = aElemKey.getAttribute("key");
      key = key.toUpperCase();
    }
    return elemString + key;
  },

  findShortcut(aElemCommand) {
    let document = aElemCommand.ownerDocument;
    return document.querySelector("key[command=\"" + aElemCommand.getAttribute("id") + "\"]");
  }
};

Object.freeze(ShortcutUtils);

this.ShortcutUtils = ShortcutUtils;
