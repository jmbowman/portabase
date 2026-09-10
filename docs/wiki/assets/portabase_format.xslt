<?xml version="1.0"?>
<xsl:stylesheet version="1.0"
     xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:template match="/">
<html>
<head>
<title>PortaBase File Contents</title>
</head>
<body>
  <h2>PortaBase File Contents</h2>
  <xsl:apply-templates select="/portabase/global"/>
  <xsl:apply-templates select="/portabase/enums"/>
  <xsl:apply-templates select="/portabase/enumoptions"/>
  <xsl:apply-templates select="/portabase/columns"/>
  <xsl:apply-templates select="/portabase/views"/>
  <xsl:apply-templates select="/portabase/viewcolumns"/>
  <xsl:apply-templates select="/portabase/sorts"/>
  <xsl:apply-templates select="/portabase/sortcolumns"/>
  <xsl:apply-templates select="/portabase/filters"/>
  <xsl:apply-templates select="/portabase/filterconditions"/>
  <xsl:apply-templates select="/portabase/calcs"/>
  <xsl:apply-templates select="/portabase/calcnodes"/>
  <p><b>data</b></p>
  <table border="1">
    <tr style="color:blue">
      <xsl:for-each select="/portabase/columns/column">
        <xsl:sort type="number" select="cid"/>
        <td>
          <xsl:choose>
            <xsl:when test="ctype='1'">
              <xsl:text>I</xsl:text><xsl:value-of select="cid"/>
            </xsl:when>
            <xsl:when test="ctype='3'">
              <xsl:text>I</xsl:text><xsl:value-of select="cid"/>
            </xsl:when>
            <xsl:when test="ctype='5'">
              <xsl:text>I</xsl:text><xsl:value-of select="cid"/>
            </xsl:when>
            <xsl:when test="ctype='6'">
              <xsl:text>I</xsl:text><xsl:value-of select="cid"/>
            </xsl:when>
            <xsl:when test="ctype='8'">
              <xsl:text>I</xsl:text><xsl:value-of select="cid"/>
            </xsl:when>
            <xsl:when test="ctype='9'">
              <xsl:text>B</xsl:text><xsl:value-of select="cid"/>
            </xsl:when>
            <xsl:otherwise>
              <xsl:text>S</xsl:text><xsl:value-of select="cid"/>
            </xsl:otherwise>
          </xsl:choose>
        </td>
      </xsl:for-each>
    </tr>
    <xsl:apply-templates select="/portabase/data"/>
  </table>
</body>
</html>
</xsl:template>

<xsl:template match="/portabase/global">
  <p><b>global</b></p>
  <table border="1">
    <tr style="color:blue">
      <xsl:for-each select="*">
        <td><xsl:value-of select="name()"/></td>
      </xsl:for-each>
    </tr>
    <tr>
      <xsl:for-each select="*">
        <td><xsl:value-of select="."/></td>
      </xsl:for-each>
    </tr>
  </table>
</xsl:template>

<xsl:template match="/portabase/columns">
  <p><b>columns</b></p>
  <table border="1">
    <tr style="color:blue">
      <td>cindex</td>
      <td>cname</td>
      <td>ctype</td>
      <td>cdefault</td>
      <td>cid</td>
    </tr>
    <tr>
      <xsl:apply-templates/>
    </tr>
  </table>
</xsl:template>

<xsl:template match="/portabase/views">
  <p><b>views</b></p>
  <table border="1">
    <tr style="color:blue">
      <td>vname</td>
      <td>vrpp</td>
      <td>vdeskrpp</td>
      <td>vsort</td>
      <td>vfilter</td>
    </tr>
    <tr>
      <xsl:apply-templates/>
    </tr>
  </table>
</xsl:template>

<xsl:template match="/portabase/viewcolumns">
  <p><b>viewcolumns</b></p>
  <table border="1">
    <tr style="color:blue">
      <td>vcview</td>
      <td>vcindex</td>
      <td>vcname</td>
      <td>vcwidth</td>
      <td>vcdeskwidth</td>
    </tr>
    <tr>
      <xsl:apply-templates/>
    </tr>
  </table>
</xsl:template>

<xsl:template match="/portabase/sorts">
  <p><b>sorts</b></p>
  <table border="1">
    <tr style="color:blue">
      <td>sname</td>
    </tr>
    <tr>
      <xsl:apply-templates/>
    </tr>
  </table>
</xsl:template>

<xsl:template match="/portabase/sortcolumns">
  <p><b>sortcolumns</b></p>
  <table border="1">
    <tr style="color:blue">
      <td>scsort</td>
      <td>scindex</td>
      <td>scname</td>
      <td>scdesc</td>
    </tr>
    <tr>
      <xsl:apply-templates/>
    </tr>
  </table>
</xsl:template>

<xsl:template match="/portabase/filters">
  <p><b>filters</b></p>
  <table border="1">
    <tr style="color:blue">
      <td>fname</td>
    </tr>
    <tr>
      <xsl:apply-templates/>
    </tr>
  </table>
</xsl:template>

<xsl:template match="/portabase/filterconditions">
  <p><b>filterconditions</b></p>
  <table border="1">
    <tr style="color:blue">
      <td>fcfilter</td>
      <td>fcposition</td>
      <td>fccolumn</td>
      <td>fcoperator</td>
      <td>fcconstant</td>
      <td>fccase</td>
    </tr>
    <tr>
      <xsl:apply-templates/>
    </tr>
  </table>
</xsl:template>

<xsl:template match="/portabase/enums">
  <p><b>enums</b></p>
  <table border="1">
    <tr style="color:blue">
      <td>ename</td>
      <td>eid</td>
      <td>eindex</td>
    </tr>
    <tr>
      <xsl:apply-templates/>
    </tr>
  </table>
</xsl:template>

<xsl:template match="/portabase/enumoptions">
  <p><b>enumoptions</b></p>
  <table border="1">
    <tr style="color:blue">
      <td>eoenum</td>
      <td>eoindex</td>
      <td>eotext</td>
    </tr>
    <tr>
      <xsl:apply-templates/>
    </tr>
  </table>
</xsl:template>

<xsl:template match="/portabase/calcs">
  <p><b>calcs</b></p>
  <table border="1">
    <tr style="color:blue">
      <td>calcid</td>
      <td>calcdecimals</td>
    </tr>
    <tr>
      <xsl:apply-templates/>
    </tr>
  </table>
</xsl:template>

<xsl:template match="/portabase/calcnodes">
  <p><b>calcnodes</b></p>
  <table border="1">
    <tr style="color:blue">
      <td>cnid</td>
      <td>cnnodeid</td>
      <td>cnparentid</td>
      <td>cntype</td>
      <td>cnvalue</td>
    </tr>
    <tr>
      <xsl:apply-templates/>
    </tr>
  </table>
</xsl:template>

<xsl:template match="column|view|viewcolumn|sort|sortcolumn|filter|filtercondition|enum|enumoption|calc|calcnode">
  <tr>
    <xsl:for-each select="*">
      <td><xsl:value-of select="."/></td>
    </xsl:for-each>
  </tr>
</xsl:template>

<xsl:template match="r">
  <tr>
    <xsl:for-each select="*">
      <xsl:sort type="number" select="@c"/>
      <xsl:choose>
        <xsl:when test="local-name(.)='p'">
          <td><a><xsl:attribute name="href"><xsl:value-of select="."/></xsl:attribute><xsl:value-of select="."/></a></td>
        </xsl:when>
        <xsl:otherwise>
          <td><xsl:value-of select="."/></td>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:for-each>
  </tr>
</xsl:template>

<xsl:template match="/portabase/data">
  <xsl:apply-templates/>
</xsl:template>

</xsl:stylesheet>
