<?xml version="1.0" encoding="UTF-8"?>
<!-- 
     File : test_include.xsl     
     Author: Keith Isdale <k_isdale@tpg.com.au>
     Description: stylesheet for include testing 
     Copyright Reserved Under GPL     
-->
<!-- This file does not require translation -->
<!-- NO TRANSLATION -->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                version="1.0">

  <xsl:template match="include_bot">
        <xsl:apply-templates/>
  </xsl:template>

</xsl:stylesheet>
