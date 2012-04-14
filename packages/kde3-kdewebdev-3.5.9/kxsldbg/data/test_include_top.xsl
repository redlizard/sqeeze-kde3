<?xml version="1.0" encoding="UTF-8"?>
<!-- 
     File : test_include_top.xsl     
     Author: Keith Isdale <k_isdale@tpg.com.au>
     Description: stylesheet for include testing 
     Copyright Reserved Under GPL     
-->
<!-- This file does not require translation -->
<!-- NO TRANSLATION -->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                version="1.0">

  <xsl:template name="import_top">
        <xsl:apply-templates select="result/head"/>
  </xsl:template>

</xsl:stylesheet>
