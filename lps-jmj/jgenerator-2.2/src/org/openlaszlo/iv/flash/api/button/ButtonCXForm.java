/*
 * $Id: ButtonCXForm.java,v 1.2 2002/02/15 23:44:28 skavish Exp $
 *
 * ===========================================================================
 *
 * The JGenerator Software License, Version 1.0
 *
 * Copyright (c) 2000 Dmitry Skavish (skavish@usa.net). All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The end-user documentation included with the redistribution, if
 *    any, must include the following acknowlegement:
 *    "This product includes software developed by Dmitry Skavish
 *     (skavish@usa.net, http://www.flashgap.com/)."
 *    Alternately, this acknowlegement may appear in the software itself,
 *    if and wherever such third-party acknowlegements normally appear.
 *
 * 4. The name "The JGenerator" must not be used to endorse or promote
 *    products derived from this software without prior written permission.
 *    For written permission, please contact skavish@usa.net.
 *
 * 5. Products derived from this software may not be called "The JGenerator"
 *    nor may "The JGenerator" appear in their names without prior written
 *    permission of Dmitry Skavish.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL DMITRY SKAVISH OR THE OTHER
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

package org.openlaszlo.iv.flash.api.button;

import java.io.PrintStream;
import org.openlaszlo.iv.flash.parser.*;
import org.openlaszlo.iv.flash.util.*;
import org.openlaszlo.iv.flash.api.*;

/**
 * This object is attached to Button. It does not live standalone.
 */
public class ButtonCXForm extends FlashObject {

    protected Button button;
    protected CXForm cxform;

    public ButtonCXForm() {}

    public int getTag() {
        return Tag.DEFINEBUTTONCXFORM;
    }

    public void setButton( Button button ) {
        this.button = button;
    }

    public static ButtonCXForm parse( Parser p ) {
        ButtonCXForm o = new ButtonCXForm();
        o.button = (Button) p.getDef( p.getUWord() );
        o.cxform = CXForm.parse(p, false);
        o.button.setButtonCXForm(o);
        return o;
    }

    public void write( FlashOutput fob ) {
        int pos = fob.getPos();
        fob.skip(2);
        fob.writeDefID( button );
        cxform.write(fob);
        fob.writeShortTagAt(getTag(), fob.getPos()-pos-2, pos);
    }

    public void printContent( PrintStream out, String indent ) {
        out.println( indent+"ButtonCXForm:" );
        cxform.printContent(out, indent+"    ");
    }

    /*  public void collectDeps( DepsCollector dc ) {
        dc.addDep(button);
      }

      protected boolean _isConstant() {
        return button.isConstant();
      }*/

    protected FlashItem copyInto( FlashItem item, ScriptCopier copier ) {
        super.copyInto( item, copier );
        ((ButtonCXForm)item).cxform = (CXForm) cxform.getCopy(copier);
        // there is no need to copy button, this object is always attached to Button
        //    ((ButtonCXForm)item).button = (Button) copier.copy(button);
        return item;
    }

    public FlashItem getCopy( ScriptCopier copier ) {
        return copyInto( new ButtonCXForm(), copier );
    }
}
