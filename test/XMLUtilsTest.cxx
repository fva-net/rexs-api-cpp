/*
 * Copyright Schaeffler Technologies AG & Co. KG (info.de@schaeffler.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <rexsapi/XmlUtils.hxx>

#include <test/TestHelper.hxx>

#include <doctest.h>


namespace
{
  const auto* const schema = R"(
    <?xml version="1.0" encoding="UTF-8"?>
    <xsd:schema xmlns:xsd="http://www.w3.org/2001/XMLSchema">
      <xsd:element name="address">
        <xsd:complexType>
          <xsd:sequence>
            <xsd:element name="name" type="xsd:string" maxOccurs="1" minOccurs="1"/>
            <xsd:element name="street" type="xsd:string" maxOccurs="1" minOccurs="1"/>
            <xsd:element name="city" type="xsd:string" maxOccurs="1" minOccurs="1"/>
            <xsd:element name="country" type="xsd:string" maxOccurs="1" minOccurs="1"/>
          </xsd:sequence>
        </xsd:complexType>
      </xsd:element>
    </xsd:schema>
   )";
}

TEST_CASE("Xml utils test")
{
  rexsapi::TBufferXsdSchemaLoader schemaLoader{schema};
  rexsapi::TXSDSchemaValidator validator{schemaLoader};
  rexsapi::TResult result;

  SUBCASE("Load valid document")
  {
    const std::string validXml = R"(
      <?xml version="1.0" encoding="UTF-8"?>
      <address>
        <name>Puschel Bert</name>
        <street>Bärchenstr. 7</street>
        <city>Brummhausen</city>
        <country>Bärnsmark</country>
      </address>
    )";
    std::vector<uint8_t> buffer{validXml.begin(), validXml.end()};
    const auto doc = rexsapi::detail::loadXMLDocument(result, buffer, validator);
    CHECK(result);
  }

  SUBCASE("Load invalid document")
  {
    const std::string invalidXml = R"(
      <?xml version="1.0" encoding="UTF-8"?>
      <address>
        <name>Puschel Bert</name>
        <city>Brummhausen</city>
        <city>Bärnsdorf</city>
        <country>Bärnsmark</country>
      </address>
    )";
    std::vector<uint8_t> buffer{invalidXml.begin(), invalidXml.end()};
    const auto doc = rexsapi::detail::loadXMLDocument(result, buffer, validator);
    CHECK_FALSE(result);
    REQUIRE(result.getErrors().size() == 2);
    CHECK(result.getErrors()[0].getMessage() == "[/address/] too few 'street' elements, found 0 instead of at least 1");
    CHECK(result.getErrors()[1].getMessage() == "[/address/] too many 'city' elements, found 2 instead of at most 1");
  }

  SUBCASE("load invalid xml")
  {
    const std::string badXml = R"(
      <?xml version="1.0" encoding="UTF-8"?>
      <address>
        <name>Puschel Bert
        <city>Brummhausen</city>
        <city>Bärnsdorf</city>
        <country>Bärnsmark</country>
      </address>
    )";
    std::vector<uint8_t> buffer{badXml.begin(), badXml.end()};
    const auto doc = rexsapi::detail::loadXMLDocument(result, buffer, validator);
    CHECK_FALSE(result);
    REQUIRE(result.getErrors().size() == 1);
    CHECK(result.getErrors()[0].getMessage() == "Start-end tags mismatch: offset 200");
  }
}

TEST_CASE("Check xml")
{
  SUBCASE("Check xml control characters in xml child pcdata")
  {
    pugi::xml_document doc;
    auto decl = doc.append_child(pugi::node_declaration);
    decl.append_attribute("version") = "1.0";
    decl.append_attribute("encoding") = "UTF-8";
    decl.append_attribute("standalone") = "no";

    auto childNode = doc.append_child("child");
    childNode.append_child(pugi::node_pcdata).set_value("<this is dump>");

    std::stringstream stream;
    doc.save(stream, "  ");

    auto result = R"(<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<child>&lt;this is dump&gt;</child>
)";
    CHECK(stream.str() == result);
  }
}
