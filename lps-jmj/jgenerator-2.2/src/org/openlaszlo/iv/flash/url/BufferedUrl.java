/*
 * $Id: BufferedUrl.java,v 1.2 2002/02/15 23:44:28 skavish Exp $
 *
 * ==========================================================================
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

package org.openlaszlo.iv.flash.url;

import java.io.*;
import java.net.*;
import java.util.*;
import org.openlaszlo.iv.flash.api.*;
import org.openlaszlo.iv.flash.util.*;

/**
 * Buffered url
 * <P>
 * This class reads data from url and caches them in a buffer
 *
 * @author Dmitry Skavish
 */
public class BufferedUrl extends IVUrl {

    private FlashBuffer fb;
    private IVUrl url;

    /**
     * Creates buffered url from specified IVUrl
     *
     * @param url    specified url
     * @exception IOException
     */
    public BufferedUrl( IVUrl url ) throws IOException {
        this.url = url;
        this.fb = Util.readUrl( url );
    }

    /**
     * Creates buffered url from specified url and flash file
     *
     * @param file    flash file in which context to create url
     * @param surl    specified url
     * @exception IOException
     */
    public BufferedUrl( String surl, FlashFile file ) throws IOException, IVException {
        this.url = IVUrl.newUrl(surl, file);
        this.fb = Util.readUrl( url );
    }

    /**
     * Creates buffered url from buffer
     *
     * @param fb     buffer of data
     */
    public BufferedUrl( FlashBuffer fb ) {
        this.fb = fb;
    }

    public FlashBuffer getFlashBuffer() {
        return fb;
    }

    public String getName() {
        return "fgbuffer:///"+hashCode();
    }

    public InputStream getInputStream() throws IOException {
        return fb.getInputStream();
    }

    public String getParameter( String name ) {
        if( url == null ) return super.getParameter( name );
        return url.getParameter( name );
    }

    public long lastModified() {
        if( url == null ) return super.lastModified();
        return url.lastModified();
    }

    public void refresh() {
        if( url == null ) {
            super.refresh();
        } else {
            url.refresh();
        }
    }

    public String getRef() {
        if( url == null ) return super.getRef();
        return url.getRef();
    }

}
